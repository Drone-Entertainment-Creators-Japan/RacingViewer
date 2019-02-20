#include "CameraVideoSource.h"
#include "models/PilotListModel.h"
#include "models/LapTimeCollection.h"
#include "models/LapTimeRecord.h"
#include "cv/RecognitionThread.h"
#include "main/Definitions.h"

#include <QtMultimedia/QCamera>
#include <QtMultimedia/QVideoSurfaceFormat>
#include <QtCore/QByteArray>
#include <QPainter>
#include <QCamera>
#include <QSound>
#include <QMutexLocker>
#include <QFontDatabase>

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

/* ------------------------------------------------------------------------------------------------ */
CameraVideoSource::CameraVideoSource(const QString& device_id, PilotListModel* p_pilot_list, LapTimeCollection* p_laptimes, QObject* p_parent) : QAbstractVideoSurface(p_parent)
, m_active         ( false )
, mp_camera        ( nullptr )
, m_name           ( tr("Unknown") )
, m_device_id      ( device_id )
, m_camera_aspect  ( Definitions::kAspect_None )
, m_image_format   (QImage::Format_Invalid)
, mp_pilot_list    ( p_pilot_list )
, mp_laptime_list  ( p_laptimes   )
, mp_laptime       ( nullptr )
, m_marker_type    ( Definitions::kTxtARMarkerType_4X4_50 )
, mp_thread        ( nullptr )
{
    if( m_device_id.isEmpty() ) { return; }
    mp_camera = new QCamera( m_device_id.toUtf8(), this );
    mp_camera->start();
    mp_camera->stop();

    mp_camera->setViewfinder(this);

    /* font */
    int id = QFontDatabase::addApplicationFont(":/BradeRunner.fft");
    QString family = QFontDatabase::applicationFontFamilies(id).at(0);
    m_pilot_name_font = QFont(family);
//    m_pilot_name_font.setItalic(true);

    id = QFontDatabase::addApplicationFont(":/NeograyRegular.otf");
    family = QFontDatabase::applicationFontFamilies(id).at(0);
    m_pilot_desc_font = QFont(family);
    m_pilot_desc_font.setItalic(true);
}

/* ------------------------------------------------------------------------------------------------ */
CameraVideoSource::~CameraVideoSource(void)
{
    setActive(false);
    delete mp_camera;
    mp_camera = nullptr;
}
/* ------------------------------------------------------------------------------------------------ */
const QString& CameraVideoSource::name(void) const
{
    return m_name;
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::setName(const QString& name)
{
    m_name = name;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
QCamera* CameraVideoSource::camera(void)
{
    return mp_camera;
}

/* ------------------------------------------------------------------------------------------------ */
QList<QVideoFrame::PixelFormat> CameraVideoSource::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    if (handleType == QAbstractVideoBuffer::NoHandle)
    {
        return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB555;
    } else
    {
        return QList<QVideoFrame::PixelFormat>();
    }
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::present(const QVideoFrame& frame)
{
    m_mutex.lock();
    m_current_frame = frame;

    m_current_frame.map(QAbstractVideoBuffer::ReadOnly);
    QImage image(
                 m_current_frame.bits(),
                 m_current_frame.width(),
                 m_current_frame.height(),
                 m_current_frame.bytesPerLine(),
                 m_image_format);

    if(    (m_current_frame.pixelFormat() == QVideoFrame::Format_RGB32)
        || (m_current_frame.pixelFormat() == QVideoFrame::Format_ARGB32) )
    {
        m_current_image = image;
    } else
    {
        bool need_resize = false;
        need_resize |= m_current_image.width() != m_current_frame.width();
        need_resize |= m_current_image.height() != m_current_frame.height();
        if( need_resize )
        {
            m_current_image = QImage(m_current_frame.width(), m_current_frame.height(), QImage::Format_ARGB32);
            m_current_image.fill(0);
        }
        QPainter painter(&m_current_image);
        painter.drawImage(0, 0, image);
    }
    m_current_frame.unmap();
    m_mutex.unlock();

    if( mp_thread ) { mp_thread->update(); }
    emit update(this);

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::setActive(bool active)
{
    QMutexLocker locker( &m_mutex );
    if( ! mp_camera ) { return false; }

    m_active = active;

    if( ! active )
    {
        mp_camera->stop();
    } else
    {
        mp_camera->start();
        if( mp_camera->error() != QCamera::NoError )
        {
            m_active = false;
            qDebug() << mp_camera->error();
            qDebug() << mp_camera->errorString();
            qDebug() <<"State:" << mp_camera->state() << " Status: " << mp_camera->status();
            mp_camera->stop();
            return false;
        }
    }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::isActive(void) const
{
    return m_active;
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::setRecognitionEnabled(bool enable)
{
    if( enable )
    {
        if( ! mp_thread )
        {
            int marker_type = cv::aruco::DICT_4X4_50;
            if     ( m_marker_type == Definitions::kTxtARMarkerType_4X4_50  ) { marker_type = cv::aruco::DICT_4X4_50;  }
            else if( m_marker_type == Definitions::kTxtARMarkerType_4X4_100 ) { marker_type = cv::aruco::DICT_4X4_100; }
            else if( m_marker_type == Definitions::kTxtARMarkerType_4X4_250 ) { marker_type = cv::aruco::DICT_4X4_250; }

            mp_thread = new RecognitionThread(this);
            mp_thread->setVideoSource(this);
            mp_thread->setDictonary( cv::aruco::getPredefinedDictionary(marker_type) );
            connect(mp_thread, SIGNAL(passedThrough(int, qint64, double)), this, SLOT(passedThrough(int, qint64, double)), Qt::QueuedConnection);
        }
        mp_thread->start();
    } else
    {
        if( mp_thread ) { mp_thread->stop(); }
    }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::recognitionEnabled(void) const
{
    if( ! mp_thread ) { return false; }
    return mp_thread->isRunning();
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::setARMarkerType(const QString& type)
{
    int marker_type = cv::aruco::DICT_4X4_50;
    if     ( type == Definitions::kTxtARMarkerType_4X4_50  ) { marker_type = cv::aruco::DICT_4X4_50;  }
    else if( type == Definitions::kTxtARMarkerType_4X4_100 ) { marker_type = cv::aruco::DICT_4X4_100; }
    else if( type == Definitions::kTxtARMarkerType_4X4_250 ) { marker_type = cv::aruco::DICT_4X4_250; }
    else { return false; }

    m_marker_type = type;
    if( mp_thread ) { mp_thread->setDictonary( cv::aruco::getPredefinedDictionary(marker_type) ); }
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void CameraVideoSource::paint(QPainter* p_painter, const QRect& target_rect)
{
    bool is_ok = false;
    QMutexLocker locker( &m_mutex );

    QSize source_size = m_current_image.size();
    if( m_camera_aspect == Definitions::kAspect__4_3 ) { source_size = QSize( 4, 3); }
    if( m_camera_aspect == Definitions::kAspect_16_9 ) { source_size = QSize(16, 9); }
    if( source_size.width()  == 0 ) { return; }
    if( source_size.height() == 0 ) { return; }

    double   rate = target_rect.width()  / static_cast<double>(source_size.width());
    double v_rate = target_rect.height() / static_cast<double>(source_size.height());
    if( rate > v_rate ) { rate = v_rate; }

    QRect inner_rect(0, 0, static_cast<int>(source_size.width()*rate), static_cast<int>(source_size.height()*rate));
    inner_rect.moveCenter( QPoint(0, 0) );

    const QTransform oldTransform = p_painter->transform();
    p_painter->translate( target_rect.center() );
    if( surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop )
    {
        p_painter->scale(1, -1);
    }

    p_painter->drawImage(inner_rect, m_current_image, m_image_viewport);
    if( recognitionEnabled() )
    {
        QVector<QPoint> corner;
        mp_thread->getDetectedParameters(nullptr, &corner);

        QPen pen(Qt::red);
        pen.setWidth(3);
        p_painter->setPen(pen);
        QPoint center(source_size.width()/2, source_size.height()/2);
        double x_rate = inner_rect.width()  / static_cast<double>(  m_current_image.size().width() );
        double y_rate = inner_rect.height() / static_cast<double>(  m_current_image.size().height() );
        for(int i=0; i<corner.count(); i+=4)
        {
            QPoint first ( (corner[i].x()-center.x())*x_rate, (corner[i].y()-center.y())*y_rate );
            QPoint second( (corner[i].x()-center.x())*x_rate, (corner[i].y()-center.y())*y_rate );
            p_painter->drawLine(first, second);
        }
        p_painter->drawLine(0, 0, 10, 10);
    }
    locker.unlock();

    p_painter->setTransform(oldTransform);
    p_painter->translate( target_rect.center() + inner_rect.topLeft() );

    /* draw pilot name */
    if( ! m_pilot_name.isEmpty() && (inner_rect.height() > 200) )
    {
        double rate = inner_rect.height()/400.0;
        QRect pilot_rect(0, 0, static_cast<int>(m_pilot_prerender.width()*rate), static_cast<int>(m_pilot_prerender.height()*rate));
        p_painter->drawImage(pilot_rect, m_pilot_prerender);
        p_painter->setPen(Qt::white);

        m_pilot_name_font.setPixelSize(static_cast<int>(23*rate));
        p_painter->setFont(m_pilot_name_font);
        p_painter->drawText(static_cast<int>(96*rate), static_cast<int>(34*rate), m_pilot_name);

        m_pilot_desc_font.setPixelSize(static_cast<int>(11*rate));
        p_painter->setFont(m_pilot_desc_font);
        p_painter->drawText(static_cast<int>( 96*rate), static_cast<int>(59*rate), "lap time");
        p_painter->drawText(static_cast<int>(275*rate), static_cast<int>(59*rate), "best");
        if( mp_laptime )
        {
            QString txt;

            int current = mp_laptime->current(cv::getTickCount());
            if( current <= 0 ) { txt = tr("---"); }
            else               { txt = QTime::fromMSecsSinceStartOfDay(current).toString("mm : ss . zzz"); }
            p_painter->drawText(static_cast<int>(170*rate), static_cast<int>(59*rate),  txt);

            int best = mp_laptime->getBestTime();
            if( best <= 0 ) { txt = tr("---"); }
            else            { txt = QTime::fromMSecsSinceStartOfDay(best).toString("mm : ss . zzz"); }
            p_painter->drawText(static_cast<int>(326*rate), static_cast<int>(59*rate), txt);
        }
    }

    p_painter->setTransform(oldTransform);
}

/* ------------------------------------------------------------------------------------------------ */
QVariant CameraVideoSource::value(int property) const
{
    switch( property )
    {
        case Definitions::kSourceName:      { return m_name;       } break;
        case Definitions::kDeviceID:        { return m_device_id;  } break;
        case Definitions::kPilotAssign:     { return m_pilot_name; } break;
        case Definitions::kGateRecognition: { return recognitionEnabled(); } break;
        case Definitions::kActiveState:     { return m_active;     } break;
        default: {} break;
    }

    return QVariant();
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::setValue(int property, const QVariant& value)
{
    switch( property )
    {
        case Definitions::kSourceName:      { m_name = value.toString();       } break;
        case Definitions::kDeviceID:        { return true; } break;
        case Definitions::kPilotAssign:     { return setPilot(value.toString()); } break;
        case Definitions::kGateRecognition: { setRecognitionEnabled( value.toBool() );  } break;
        case Definitions::kActiveState:     { setActive(value.toBool()); } break;
        default: { return false; } break;
    }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
Qt::ItemFlags CameraVideoSource::itemFlags(int property) const
{
    switch( property )
    {
        case Definitions::kSourceName:      { return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable; } break;
        case Definitions::kDeviceID:        { return Qt::ItemIsSelectable; } break;
        case Definitions::kPilotAssign:     { return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable; } break;
        case Definitions::kGateRecognition: { return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable; } break;
        case Definitions::kActiveState:     { return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable; } break;
        default: {} break;
    }

    return Qt::NoItemFlags;
}

/* ------------------------------------------------------------------------------------------------ */
QString CameraVideoSource::deviceID(void) const
{
    return m_device_id;
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::getImage(cv::Mat& image, bool* p_is_bottom_to_top)
{
    QMutexLocker locker( &m_mutex );
    if( ! mp_camera ) { return false; }
    if( ! m_active  ) { return false; }

    bool is_ok = false;
    QImage::Format format = m_current_image.format();
    is_ok |= (format == QImage::Format_RGB32);
    is_ok |= (format == QImage::Format_ARGB32);
    if( ! is_ok ) { return false; }

    bool same = true;
    same &= (image.rows == m_current_frame.height() );
    same &= (image.cols == m_current_frame.width()  );
    same &= (image.type() == CV_8UC1);
    if( ! same ) { image.create(m_current_frame.height(), m_current_frame.width(), CV_8UC1); }

    const uchar* p_src = m_current_image.bits();
    uchar*       p_dst = image.data;
    uchar*       p_dst_end = image.data + image.total();
    if( (! p_src) || (! p_dst) ) { return false; }

    for(; p_dst < p_dst_end; p_dst+=1, p_src+=4)
    {
        *p_dst = static_cast<uchar>( qGray(p_src[1], p_src[2],p_src[3]) );
    }

    if( p_is_bottom_to_top ) { *p_is_bottom_to_top = (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop);}

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void CameraVideoSource::pilotDescChanged(QString pilot, int descriptor_idx, QVariant value)
{
    if( m_pilot_name.isEmpty() ) { return; }
    if( pilot != m_pilot_name  ) { return; }

    switch( descriptor_idx )
    {
//        case Definitions::kPilotName:         { m_pilot_name = value.value<QString>(); } break;
        case Definitions::kPilotImage:        { m_pilot_image = value.value<QImage>(); } break;
        case Definitions::kPilotColor:        { m_pilot_color = value.value<QColor>(); } break;
        case Definitions::kCameraAspectRatio: { m_camera_aspect = cameraAspect(value.value<QString>()); } break;
        default: {} break;
    }

    prerenderPilotDisplay();

}

/* ------------------------------------------------------------------------------------------------ */
void CameraVideoSource::passedThrough(int id, qint64 tick_count, double tick_frequency)
{
    if( m_pilot_name.isEmpty() ) { return; }
    emit passedThrough(m_pilot_name, id, tick_count, tick_frequency);
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::start(const QVideoSurfaceFormat &format)
{
    const QImage::Format image_format = QVideoFrame::imageFormatFromPixelFormat(format.pixelFormat());
    const QSize size = format.frameSize();

    if( size.isEmpty() ) { return false; }
//    if( m_image_format == QImage::Format_Invalid ) { return false; }

    m_image_size     = size;
    m_image_format   = image_format;
    m_image_viewport = format.viewport();

    QAbstractVideoSurface::start(format);

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void CameraVideoSource::stop(void)
{
    QAbstractVideoSurface::stop();
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::setPilot(const QString& name)
{
    m_pilot_name = "";

    if( ! mp_pilot_list ) { return false; }
    disconnect(mp_pilot_list, SIGNAL(pilotDescChanged(QString, int, QVariant)), this, SLOT(pilotDescChanged(QString, int, QVariant)));

    for(int idx=0; idx<mp_pilot_list->rowCount(); ++idx)
    {
        if( name != mp_pilot_list->valueOf(idx, Definitions::kPilotName).toString() ) { continue; }

        connect(mp_pilot_list, SIGNAL(pilotDescChanged(QString, int, QVariant)), this, SLOT(pilotDescChanged(QString, int, QVariant)));
        m_pilot_name = name;
        m_pilot_image = mp_pilot_list->valueOf(idx, Definitions::kPilotImage).value<QImage>();
        m_pilot_color = mp_pilot_list->valueOf(idx, Definitions::kPilotColor).value<QColor>();

        QString aspect = mp_pilot_list->valueOf(idx, Definitions::kCameraAspectRatio).value<QString>();
        m_camera_aspect = cameraAspect(aspect);

        if( mp_laptime ) { mp_laptime->cancelLap(); }
        if( mp_laptime_list ) { mp_laptime = mp_laptime_list->laptime(m_pilot_name); }
        prerenderPilotDisplay();
        return true;
    }

    if( mp_laptime ) { mp_laptime->cancelLap(); }
    mp_laptime = nullptr;
    return false;
}

/* ------------------------------------------------------------------------------------------------ */
bool CameraVideoSource::prerenderPilotDisplay(void)
{
    QImage base_image(":/PilotDisplay1_bg.png");
    QImage line_mask (":/PilotDisplay1_line.png");
    QImage pilot_mask(":/PilotDisplay1_mask.png");
    QImage pilot_image( pilot_mask.size(), QImage::Format_ARGB32 );
    QImage line_image ( line_mask.size() , QImage::Format_ARGB32 );
    pilot_image.fill( 0 );
    line_image.fill(m_pilot_color);

    { /* color bar */
        QPainter painter(&line_image);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawImage(0, 0, line_mask);
    }

    { /* pilot image */
        QPainter painter(&pilot_image);
        painter.drawImage(pilot_image.rect(), m_pilot_image, m_pilot_image.rect());
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.drawImage(0, 0, pilot_mask);
    }

    { /* prerender image */
        QPainter painter(&base_image);
        painter.drawImage(0, 0, line_image);
        painter.drawImage(17, 6, pilot_image);
    }
    m_pilot_prerender = base_image;

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
int CameraVideoSource::cameraAspect(const QString& text) const
{
    int result = Definitions::kAspect_None;
    if( text == Definitions::kTxtAspect__4_3 ) { result = Definitions::kAspect__4_3; }
    if( text == Definitions::kTxtAspect_16_9 ) { result = Definitions::kAspect_16_9; }

    return result;
}

