#include "ImageItemDelegate.h"
#include "main/Definitions.h"
#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
ImageItemDelegate::ImageItemDelegate(QObject* p_parent) : QAbstractItemDelegate(p_parent)
{
}

/* ------------------------------------------------------------------------------------------------ */
QWidget* ImageItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return nullptr;
}

/* ------------------------------------------------------------------------------------------------ */
void ImageItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    /* nothing */
}

/* ------------------------------------------------------------------------------------------------ */
void ImageItemDelegate::paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QImage image = index.data().value<QImage>();

    if( image.width()  == 0 ) { return; }
    if( image.height() == 0 ) { return; }
    double   rate = option.rect.width()  / static_cast<double>(image.width());
    double v_rate = option.rect.height() / static_cast<double>(image.height());
    if( rate > v_rate ) { rate = v_rate; }

    QRect inner_rect(0, 0, static_cast<int>(image.width()*rate), static_cast<int>(image.height()*rate));
    inner_rect.moveCenter( option.rect.center() );

    if( p_painter ) { p_painter->drawImage(inner_rect, image); }
}

/* ------------------------------------------------------------------------------------------------ */
QSize ImageItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(30, 30);
}

