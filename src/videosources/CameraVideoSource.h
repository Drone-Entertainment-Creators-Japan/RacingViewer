#ifndef CAMERAVIDEOSOURCE_H
#define CAMERAVIDEOSOURCE_H

#include "videosources/IVideoSource.h"
#include "cv/ICVImageSource.h"
#include <QtMultimedia/QAbstractVideoSurface>
#include <QtMultimedia/QVideoFrame>
#include <QString>
#include <QMutex>
#include <QRect>
#include <QSize>
#include <QFont>
#include <QImage>

class QCamera;
class PilotListModel;
class RecognitionThread;
class LapTimeCollection;
class LapTimeRecord;

class CameraVideoSource : public QAbstractVideoSurface, public IVideoSource, public ICVImageSource
{
    Q_OBJECT

public:
    explicit CameraVideoSource(const QString& device_id, PilotListModel* p_pilot_list, LapTimeCollection* p_laptimes, QObject* p_parent =nullptr);
            ~CameraVideoSource(void) override;

    const QString& name(void) const;
    bool        setName(const QString& name);

    QCamera* camera(void);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const override;

    bool present(const QVideoFrame& frame) override;
    bool setActive(bool active);
    bool  isActive(void) const;
    bool setRecognitionEnabled(bool enable);
    bool    recognitionEnabled(void) const;

    bool setARMarkerType(const QString& type);

    void paint(QPainter* p_painter, const QRect& target_rect) override;
    QVariant  value(int property) const override;
    bool   setValue(int property, const QVariant& value) override;
    Qt::ItemFlags itemFlags(int property) const override;
    QString deviceID(void) const override;

    bool getImage(cv::Mat& image) override;

public slots:
    void pilotDescChanged(QString pilot, int descriptor_idx, QVariant value);
    void passedThrough(int id, qint64 tick_count, qint64 tick_frequency);

private:
    bool start(const QVideoSurfaceFormat& format) override;
    void stop(void) override;

    bool setPilot(const QString& name);
    bool prerenderPilotDisplay(void);

    int  cameraAspect(const QString& text) const;
signals:
    void update(CameraVideoSource* p_render);
    void passedThrough(const QString& pilot, int id, qint64 tick_count, qint64 tick_frequency);

private:
    bool           m_active;
    QCamera*       mp_camera;
    QString        m_name;
    QString        m_device_id;
    QString        m_pilot_name;
    QImage         m_pilot_image;
    QColor         m_pilot_color;
    QImage         m_pilot_prerender;
    QFont          m_pilot_name_font;
    QFont          m_pilot_desc_font;
    int            m_camera_aspect;
    QSize          m_image_size;
    QRect          m_image_viewport;
    QImage::Format m_image_format;
    QVideoFrame    m_current_frame;

    PilotListModel*    mp_pilot_list;
    LapTimeCollection* mp_laptime_list;
    LapTimeRecord*     mp_laptime;
    QMutex             m_mutex;

    QString            m_marker_type;
    RecognitionThread* mp_thread;
};

#endif /* CAMERAVIDEOSOURCE_H */
