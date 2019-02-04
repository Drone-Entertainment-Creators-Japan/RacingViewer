#ifndef VIEWWINDOW_H
#define VIEWWINDOW_H

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QMetaEnum>

class QTimer;
class QSettings;
class IVideoSource;
class ViewRect;
class QPropertyAnimation;

/* ------------------------------------------------------------------------------------------------ */
class ViewWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ViewWindow(QSettings* p_settings, QWidget* p_parent=nullptr);
            ~ViewWindow(void) override;

    bool setFixedLayout(const QVector<QRectF> layout, bool is_pixel=false);
    bool setFixedCutIn (const QVector<QRectF> cutin,  bool is_pixel=false);

    bool setVideoSources(const QVector<IVideoSource*>& list);
    bool setVideoSource (int index, IVideoSource* p_render);

    bool    delVideoSource(IVideoSource* p_target);
    bool selectVideoSource(IVideoSource* p_item);

    bool setCutIn(IVideoSource* p_cutin);

signals:
    void mousePressed(IVideoSource* p_target);
    void doubleClicked(IVideoSource* p_target);

public slots:
    void timeout(void);
    void afterRemove(void);

private:
    void paintEvent(QPaintEvent*   p_event) override;
    void resizeEvent(QResizeEvent* p_event) override;

    void mouseDoubleClickEvent(QMouseEvent* p_event) override;
    void mousePressEvent(QMouseEvent* p_event) override;

    bool updateLayout(QVector<ViewRect*>& layouts);

    QRect toRect(const QRectF rectf, bool is_pixel_size);
private:
    QMetaEnum                  m_options;
    QSettings*                 mp_settings;
    IVideoSource*              m_selected;
    int                        m_pixel_margin;
    QVector<QRectF>            m_fixed_layout;
    bool                       m_fixed_layout_is_pixel;
    QVector<QRectF>            m_fixed_cutin;
    bool                       m_fixed_cutin_is_pixel;
    QVector<ViewRect*>         m_layouts;
    QRectF                     m_cutin_rect;
    ViewRect*                  mp_cutin;
    QTimer*                    mp_timer;
};

/* ------------------------------------------------------------------------------------------------ */
class ViewRect : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)
public:
    explicit ViewRect(const QRect& rect=QRect(), QObject* p_parent=0);

    bool isActive(void) const;

    bool           moveTo(const QRect& rect);
    bool      setGeometry(const QRect& rect);
    const QRect& geometry(void) const;

    bool       setSource(IVideoSource* p_render);
    IVideoSource* source(void);

    bool         isMoving(void) const;
signals:
    void finished(void);

private slots:
    void moving_is_finished(void);

private:
    QRect               m_rect;
    IVideoSource*       mp_source;
    QPropertyAnimation* mp_animation;
};

#endif /* VIEWWINDOW_H */
