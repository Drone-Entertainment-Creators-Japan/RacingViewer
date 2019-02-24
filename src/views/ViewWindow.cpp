#include "ViewWindow.h"
#include "videosources/IVideoSource.h"
#include "main/Definitions.h"
#include <QPainter>
#include <QTimer>
#include <QSettings>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <algorithm>
#include <math.h>

/* ------------------------------------------------------------------------------------------------ */
ViewWindow::ViewWindow(QSettings* p_settings, QWidget *parent) : QWidget(parent)
, mp_settings    ( p_settings )
, m_selected     ( nullptr )
, m_pixel_margin ( 2 )
, m_fixed_layout_is_pixel( false )
, m_fixed_cutin  ( 3 )
, m_fixed_cutin_is_pixel ( false )
, mp_cutin       ( nullptr )
, mp_timer       ( nullptr )
, m_view_load    ( false   )
{
    setFixedCutIn(QVector<QRectF>(), false);

    const QMetaObject& defs = Definitions::staticMetaObject;
    m_options = defs.enumerator( defs.indexOfEnumerator("Options") );

    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowFullscreenButtonHint);
    setAutoFillBackground(false);

    int fps = 0;
    if( mp_settings ) { fps = mp_settings->value(m_options.valueToKey(Definitions::kViewerFPS)).value<int>(); }
    if( fps < 15 ) { fps = 20; }

    mp_timer = new QTimer(this);
    connect(mp_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    mp_timer->setSingleShot(false);
    mp_timer->setInterval(1000/fps);
    mp_timer->start();
}

/* ------------------------------------------------------------------------------------------------ */
ViewWindow::~ViewWindow(void)
{
    delete mp_timer;
    for(int i=0; i<m_layouts.count(); ++i)
    {
        if( m_layouts[i] == mp_cutin ) { mp_cutin = nullptr; }
        delete m_layouts[i];
    }
    delete mp_cutin;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::setViewLoad(bool enabled)
{
    m_view_load = enabled;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::setFixedLayout(const QVector<QRectF> layout, bool is_pixel)
{
    m_fixed_layout = layout;
    m_fixed_layout_is_pixel = is_pixel;

    updateLayout(m_layouts);

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::setFixedCutIn(const QVector<QRectF> cutin, bool is_pixel)
{
    if( cutin.count() < 3 )
    {
        m_fixed_cutin.resize(3);
        m_fixed_cutin[0] = QRectF(1.0, 1.0, 0.0, 0.0);
        m_fixed_cutin[1] = QRectF(0.0, 0.0, 1.0, 1.0);
        m_fixed_cutin[2] = QRectF(1.0, 1.0, 0.0, 0.0);
        m_fixed_cutin_is_pixel = false;
    } else
    {
        m_fixed_cutin = cutin;
        m_fixed_cutin_is_pixel = is_pixel;
    }

    updateLayout(m_layouts);

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::setVideoSources(const QVector<IVideoSource*>& list)
{
    if( list.empty() )
    {
        for(int i=0; i<m_layouts.count(); ++i) { delete m_layouts[i]; }
        m_layouts.clear();
    }

    QVector<ViewRect*> new_layouts(list.count());
    for(int rnd=0; rnd<new_layouts.count(); ++rnd)
    {
        for(int lyt=0; lyt<m_layouts.count(); ++lyt)
        {
            if( ! m_layouts[lyt] ) { continue; }
            if( m_layouts[lyt]->source() != list[rnd]) { continue; }
            new_layouts[rnd] = m_layouts[lyt];
            m_layouts[lyt] = nullptr;
        }
    }
    for(int i=0; i<m_layouts.count(); ++i) { delete m_layouts[i]; }

    m_layouts = new_layouts;
    for(int i=0; i<m_layouts.count(); ++i)
    {
        if( ! m_layouts[i] ) { m_layouts[i] = new ViewRect(); }
        m_layouts[i]->setSource(list[i]);
    }

    updateLayout(m_layouts);
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::setVideoSource(int index, IVideoSource* p_render)
{
    if( index <  0 ) { return false; }
    if( index >= m_layouts.count() ) { m_layouts.resize(index+1); }

    if( ! m_layouts[index] ) { m_layouts[index] = new ViewRect(); }
    m_layouts[index]->setSource(p_render);

    if( ! p_render )
    {
        int count=0;
        for(count=m_layouts.count()-1; count >= 0; --count)
        {
            if( ! m_layouts[count] ) { continue; }
            if( m_layouts[count]->source() ) { break; }
        }
        count += 1;
        for(int i=count; i<m_layouts.count(); ++i) { delete m_layouts[i]; }
        m_layouts.resize(count);
    }

    updateLayout( m_layouts );
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::delVideoSource(IVideoSource* p_target)
{
    if( ! p_target ) { return false; }
    int idx = 0;
    for(; idx < m_layouts.count(); ++idx) { if( p_target == m_layouts[idx]->source() ) { break; } }
    if( idx >= m_layouts.count() ) { return true; }

    if( mp_cutin == m_layouts[idx] ) { mp_cutin = nullptr; }
    delete m_layouts[idx];
    m_layouts.remove(idx);

    updateLayout(m_layouts );

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::selectVideoSource(IVideoSource* p_item)
{
    m_selected = p_item;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::setCutIn(IVideoSource* p_cutin)
{
    if( m_fixed_cutin.count() < 3 ) { return false; }

    if( p_cutin ) { m_cutin_rect = m_fixed_cutin[1]; }
    else          { m_cutin_rect = m_fixed_cutin[2]; }

    if( m_layouts.indexOf(mp_cutin) >= 0 ) { mp_cutin = nullptr; };
    if( (! p_cutin) && mp_cutin ) { connect(mp_cutin, SIGNAL(finished()), this, SLOT(afterRemove())); }

    for(int i=0; i<m_layouts.count(); ++i)
    {
        if( ! m_layouts[i] ) { continue; }
        if( ! m_layouts[i]->source() ) { continue; }
        if( m_layouts[i]->source() != p_cutin ) { continue; }
        delete mp_cutin;
        mp_cutin = m_layouts[i];
        break;
    }

    if( p_cutin && (! mp_cutin) ) { mp_cutin = new ViewRect( toRect(m_fixed_cutin[0], m_fixed_cutin_is_pixel) ); }
    if( p_cutin &&   mp_cutin   ) { mp_cutin->setSource(p_cutin); }

    updateLayout( m_layouts );

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::timeout(void)
{
    int fps = 0;
    if( mp_settings ) { fps = mp_settings->value(m_options.valueToKey(Definitions::kViewerFPS)).value<int>(); }
    if( fps < 15 ) { fps = 20; }

    if( mp_timer ) { mp_timer->setInterval(1000/fps); }

    update();
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::afterRemove(void)
{
    disconnect(mp_cutin, SIGNAL(finished()), this, SLOT(afterRemove()));
    if( m_layouts.indexOf(mp_cutin) >= 0 ) { mp_cutin = nullptr; };
    delete mp_cutin;
    mp_cutin = nullptr;
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::changeEvent(QEvent* p_event)
{
    if( ! p_event ) { return; }
    if( p_event->type() == QEvent::WindowStateChange )
    {
        if( this->windowState() == Qt::WindowMaximized )
        {
            setWindowState( Qt::WindowFullScreen );
        }
    }
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::keyPressEvent(QKeyEvent* p_event)
{
    if( ! p_event ) { return; }
    if( this->windowState() != Qt::WindowFullScreen ) { return; }
    if( p_event->key() != Qt::Key_Escape ) { return; }

    setWindowState( Qt::WindowNoState );
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::paintEvent(QPaintEvent* p_event)
{
    QPainter painter(this);

    painter.fillRect(rect(), QBrush(Qt::black));
    painter.setPen(Qt::red);

    int count = m_layouts.count();
    QRect inner_rect;
    painter.setPen(Qt::blue);

    /* already moved video sources*/
    for(int i=0; i<count; ++i)
    {
        if( ! m_layouts[i] ) { continue; }
        if( m_layouts[i] == mp_cutin ) { continue; }
        IVideoSource* p_render = m_layouts[i]->source();
        if( ! p_render                 ) { continue; }
        if( ! m_layouts[i]->isActive() ) { continue; }
        if(   m_layouts[i]->isMoving() ) { continue; }

        p_render->paint(&painter, m_layouts[i]->geometry(), &inner_rect);
        painter.setPen(Qt::white);
        int load = p_render->recognitionLoad();
        if( (load > 0) && m_view_load )
        {
            painter.setFont( QFont() );
            if( load > 80 ) { painter.setPen(Qt::red); }
            QString text;
            text.sprintf("% 3d%%", load);
            QRect bound_rect = painter.boundingRect(inner_rect, Qt::AlignLeft | Qt::AlignBottom, text);
            painter.fillRect(bound_rect, QBrush( QColor(0u, 0u, 0u, 200u)));
            painter.drawText(bound_rect, text); }
        if( p_render != m_selected ) { continue; }
        painter.drawRect(inner_rect);
    }

    /* moving video sources */
    for(int i=0; i<count; ++i)
    {
        if( ! m_layouts[i] ) { continue; }
        if( m_layouts[i] == mp_cutin ) { continue; }
        IVideoSource* p_render = m_layouts[i]->source();
        if( ! p_render                 ) { continue; }
        if( ! m_layouts[i]->isActive() ) { continue; }
        if( ! m_layouts[i]->isMoving() ) { continue; }

        p_render->paint(&painter, m_layouts[i]->geometry());
    }

    if( ! mp_cutin ) { return; }
    if( ! mp_cutin->source() ) { return; }
    if( ! mp_cutin->isActive() ) { return; }
    mp_cutin->source()->paint(&painter, mp_cutin->geometry());
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::resizeEvent(QResizeEvent* p_event)
{
    updateLayout( m_layouts );
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::mouseDoubleClickEvent(QMouseEvent* p_event)
{
    if( ! p_event ) { return; }

    QPoint pos = p_event->pos();

    ViewRect* p_target = nullptr;
    for(int i=0; i<m_layouts.count(); ++i)
    {
        if( ! m_layouts[i] ) { continue; }
        if( ! m_layouts[i]->source() ) { continue; }

        if( ! m_layouts[i]->geometry().contains(pos) ) { continue; }
        p_target = m_layouts[i];
    }
    if( mp_cutin )
    {
        if( mp_cutin->source() && mp_cutin->geometry().contains(pos) )
        {
            p_target = mp_cutin;
        }
    }

    if( p_target ) { emit doubleClicked(p_target->source()); }
}

/* ------------------------------------------------------------------------------------------------ */
void ViewWindow::mousePressEvent(QMouseEvent* p_event)
{
    if( ! p_event ) { return; }

    QPoint pos = p_event->pos();

    ViewRect* p_target = nullptr;
    for(int i=0; i<m_layouts.count(); ++i)
    {
        if( ! m_layouts[i] ) { continue; }
        if( ! m_layouts[i]->source() ) { continue; }

        if( ! m_layouts[i]->geometry().contains(pos) ) { continue; }
        p_target = m_layouts[i];
    }
    if( mp_cutin )
    {
        if( mp_cutin->source() && mp_cutin->geometry().contains(pos) )
        {
            p_target = mp_cutin;
        }
    }

    if( p_target ) { emit mousePressed(p_target->source()); }
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewWindow::updateLayout(QVector<ViewRect*>& layouts)
{
    static const double kAspect = 3.0/4.0;
    if( this->width()  == 0 ) { return false; }
    if( this->height() == 0 ) { return false; }

    int item_count = layouts.count();

    if( ! m_fixed_layout.isEmpty() )
    {
        int layout_count=std::min(m_fixed_layout.count(), item_count);
        int i=0;
        for(; i<layout_count; ++i)
        {
            if( layouts[i] == mp_cutin ) { continue; }

            layouts[i]->moveTo(toRect(m_fixed_layout[i], m_fixed_layout_is_pixel));
        }
        for(; i<item_count; ++i)
        {
            if( ! layouts[i] ) { continue; }
            if( layouts[i] == mp_cutin ) { continue; }

            QRect rect = layouts[i]->geometry();
            QPoint center = rect.center();
            rect.setX( center.x() );
            rect.setY( center.y() );
            rect.setWidth ( 0 );
            rect.setHeight( 0 );
            layouts[i]->moveTo(rect);
        }

    } else
    {
        double window_rate = this->height()/static_cast<double>(this->width()) / kAspect;
        int max_h_count = 1;
        double max_rate = 0;
        for(int h_count=1; h_count <= item_count; ++h_count)
        {
            double h_rate = 1/static_cast<double>(h_count);
            double v_rate = 1/ceil(item_count / static_cast<double>(h_count)) * window_rate;
            double rate = std::min(h_rate, v_rate);
            if( rate < max_rate ) { continue; }
            max_rate = rate;
            max_h_count = h_count;
        }
        int max_v_count = static_cast<int>( ceil(item_count / static_cast<double>(max_h_count)) );

        int width  = static_cast<int>( 1/static_cast<double>(max_h_count) * this->width()  );
        int height = static_cast<int>( 1/static_cast<double>(max_v_count) * this->height() );
        for(int i=0; i<item_count; ++i)
        {
            if( ! layouts[i] ) { continue; }
            if( layouts[i] == mp_cutin ) { continue; }

            QRect rect;
            rect.setX( (i % max_h_count) * width  + m_pixel_margin);
            rect.setY( (i / max_h_count) * height + m_pixel_margin);
            rect.setWidth (width  - m_pixel_margin);
            rect.setHeight(height - m_pixel_margin);
            layouts[i]->moveTo(rect);
        }
    }

    if( ! mp_cutin ) { return true; }
    mp_cutin->moveTo( toRect(m_cutin_rect, m_fixed_cutin_is_pixel) );

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
QRect ViewWindow::toRect(const QRectF rectf, bool is_pixel_size)
{
    int width  = this->width();
    int height = this->height();
    if( m_fixed_layout_is_pixel )
    {
        width  = 1;
        height = 1;
    }

    QRect rect;
    rect.setX( static_cast<int>( width  * rectf.x() ) + m_pixel_margin);
    rect.setY( static_cast<int>( height * rectf.y() ) + m_pixel_margin);
    rect.setWidth ( static_cast<int>( width  * rectf.width()  ) - m_pixel_margin);
    rect.setHeight( static_cast<int>( height * rectf.height() ) - m_pixel_margin);

    return rect;
}




/* ------------------------------------------------------------------------------------------------ */
ViewRect::ViewRect(const QRect& rect, QObject* p_parent) : QObject(p_parent)
, m_rect      ( rect )
, mp_source   ( nullptr )
, mp_animation( nullptr )
{

}

/* ------------------------------------------------------------------------------------------------ */
bool ViewRect::isActive(void) const
{
    if( m_rect.width()  <= 0 ) { return false; }
    if( m_rect.height() <= 0 ) { return false; }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewRect::moveTo(const QRect& rect)
{
    if( m_rect == rect ) { return true; }
    if( ! mp_animation )
    {
        mp_animation = new QPropertyAnimation(this, "geometry");
        mp_animation->setEasingCurve( QEasingCurve(QEasingCurve::InOutQuad) );
        connect(mp_animation, SIGNAL(finished()), this, SLOT(moving_is_finished()));
    }
    mp_animation->setDuration(700);
    mp_animation->setStartValue( this->geometry() );
    mp_animation->setEndValue( rect );

    mp_animation->start();

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewRect::setGeometry(const QRect& rect)
{
    m_rect = rect;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
const QRect& ViewRect::geometry(void) const
{
    return m_rect;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewRect::setSource(IVideoSource* p_source)
{
    mp_source = p_source;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
IVideoSource* ViewRect::source(void)
{
    return mp_source;
}

/* ------------------------------------------------------------------------------------------------ */
bool ViewRect::isMoving(void) const
{
    if( ! mp_animation ) { return false; }
    return mp_animation->state() == QPropertyAnimation::Running;
}

/* ------------------------------------------------------------------------------------------------ */
void ViewRect::moving_is_finished(void)
{
    emit finished();
}
