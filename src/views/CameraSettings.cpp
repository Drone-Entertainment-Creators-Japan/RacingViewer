#include "CameraSettings.h"
#include "ui_CameraSettings.h"

#include <QtMultimedia/QCamera>
#include <QtMultimedia/QCameraViewfinderSettings>
#include <QMetaEnum>


QString pixelformatToString(int value);

/* ------------------------------------------------------------------------------------------------ */
CameraSettings::CameraSettings(QCamera* p_target, QWidget* p_parent) : QDialog(p_parent)
, mp_ui(new Ui::CameraSettings)
, mp_target( p_target )
{
    mp_ui->setupUi(this);

    if( ! mp_target ) { return; }
    QCameraViewfinderSettings settings = mp_target->viewfinderSettings();
    QString text;
    QList<QSize> size_list = mp_target->supportedViewfinderResolutions();
    foreach(const QSize& item, size_list)
    {
        text.sprintf("%d x %d", item.width(), item.height());
        mp_ui->p_size->addItem( text );
    }
    mp_ui->p_size->setCurrentIndex( size_list.indexOf(settings.resolution()));

    QList<QCamera::FrameRateRange> rate_list = mp_target->supportedViewfinderFrameRateRanges();
    foreach(const QCamera::FrameRateRange& item, rate_list)
    {
        if( item.minimumFrameRate == item.maximumFrameRate ) { text.sprintf("%.3f", item.minimumFrameRate); }
        else                                                 { text.sprintf("%.3f - %.3f", item.minimumFrameRate, item.maximumFrameRate); }
        mp_ui->p_framerate->addItem( text );
    }
    if( settings.minimumFrameRate() == settings.maximumFrameRate() ) { text.sprintf("%.3f", settings.minimumFrameRate()); }
    else                                                             { text.sprintf("%.3f - %.3f", settings.minimumFrameRate(), settings.maximumFrameRate()); }
    mp_ui->p_framerate->setCurrentText(text);

    QList<QVideoFrame::PixelFormat> format_list = mp_target->supportedViewfinderPixelFormats();
    foreach(const QVideoFrame::PixelFormat& item, format_list)
    {
        mp_ui->p_format->addItem( pixelformatToString(item) );
    }
    mp_ui->p_format->setCurrentIndex( format_list.indexOf(settings.pixelFormat()) );

    connect(mp_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accepted()));
}

/* ------------------------------------------------------------------------------------------------ */
CameraSettings::~CameraSettings(void)
{
    delete mp_ui;
}

/* ------------------------------------------------------------------------------------------------ */
void CameraSettings::accepted(void)
{
    if( ! mp_target ) { return; }
    QCameraViewfinderSettings settings = mp_target->viewfinderSettings();

    int index = 0;
    QList<QSize> size_list = mp_target->supportedViewfinderResolutions();
    index = mp_ui->p_size->currentIndex();
    if( index < size_list.count() )settings.setResolution( size_list[index] );

    QList<QCamera::FrameRateRange> rate_list = mp_target->supportedViewfinderFrameRateRanges();
    index = mp_ui->p_framerate->currentIndex();
    if( index < rate_list.count() )
    {
        settings.setMinimumFrameRate( rate_list[index].minimumFrameRate );
        settings.setMaximumFrameRate( rate_list[index].maximumFrameRate );
    }

    QList<QVideoFrame::PixelFormat> format_list = mp_target->supportedViewfinderPixelFormats();
    index = mp_ui->p_format->currentIndex();
    if( index < format_list.count() )settings.setPixelFormat( format_list[index] );

    mp_target->setViewfinderSettings( settings );
}

/* ------------------------------------------------------------------------------------------------ */
QString pixelformatToString(int value)
{
    switch(value)
    {
        case QVideoFrame::Format_Invalid:                { return "Invalid"; } break;
        case QVideoFrame::Format_ARGB32:                 { return "ARGB32";  } break;
        case QVideoFrame::Format_ARGB32_Premultiplied:   { return "ARGB32 Premultiplied"; } break;
        case QVideoFrame::Format_RGB32:                  { return "RGB32"; } break;
        case QVideoFrame::Format_RGB24:                  { return "RGB24"; } break;
        case QVideoFrame::Format_RGB565:                 { return "RGB565"; } break;
        case QVideoFrame::Format_RGB555:                 { return "RGB555"; } break;
        case QVideoFrame::Format_ARGB8565_Premultiplied: { return "ARGB8565 Premultiplied"; } break;
        case QVideoFrame::Format_BGRA32:                 { return "BGRA32"; } break;
        case QVideoFrame::Format_BGRA32_Premultiplied:   { return "BGRA32 Premultiplied"; } break;
        case QVideoFrame::Format_BGR32:                  { return "BGR32"; } break;
        case QVideoFrame::Format_BGR24:                  { return "BGR24"; } break;
        case QVideoFrame::Format_BGR565:                 { return "BGR565"; } break;
        case QVideoFrame::Format_BGR555:                 { return "BGR555"; } break;
        case QVideoFrame::Format_BGRA5658_Premultiplied: { return "BGRA5658 Premultiplied"; } break;
        case QVideoFrame::Format_AYUV444:                { return "AYUV444"; } break;
        case QVideoFrame::Format_AYUV444_Premultiplied:  { return "AYUV444 Premultiplied"; } break;
        case QVideoFrame::Format_YUV444:                 { return "YUV444"; } break;
        case QVideoFrame::Format_YUV420P:                { return "YUV420P"; } break;
        case QVideoFrame::Format_YV12:                   { return "YV12"; } break;
        case QVideoFrame::Format_UYVY:                   { return "UYVY"; } break;
        case QVideoFrame::Format_YUYV:                   { return "YUYV"; } break;
        case QVideoFrame::Format_NV12:                   { return "NV12"; } break;
        case QVideoFrame::Format_NV21:                   { return "NV21"; } break;
        case QVideoFrame::Format_IMC1:                   { return "IMC1"; } break;
        case QVideoFrame::Format_IMC2:                   { return "IMC2"; } break;
        case QVideoFrame::Format_IMC3:                   { return "IMC3"; } break;
        case QVideoFrame::Format_IMC4:                   { return "IMC4"; } break;
        case QVideoFrame::Format_Y8:                     { return "Y8"; } break;
        case QVideoFrame::Format_Y16:                    { return "Y16"; } break;
        case QVideoFrame::Format_Jpeg:                   { return "Jpeg"; } break;
        case QVideoFrame::Format_CameraRaw:              { return "CameraRaw"; } break;
        case QVideoFrame::Format_AdobeDng:               { return "AdobeDng"; } break;
        case QVideoFrame::Format_User:                   { return "User"; } break;
        default: {} break;
    }

    return "Unknown";
}


