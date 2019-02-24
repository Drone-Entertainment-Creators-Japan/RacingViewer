#include "ColorItemDelegate.h"
#include "main/Definitions.h"
#include <QColorDialog>
#include <QImage>
#include <QPainter>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
ColorItemDelegate::ColorItemDelegate(QObject* p_parent) : QAbstractItemDelegate(p_parent)
{
}

/* ------------------------------------------------------------------------------------------------ */
QWidget* ColorItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return nullptr;
}

/* ------------------------------------------------------------------------------------------------ */
void ColorItemDelegate::setEditorData(QWidget* p_editor, const QModelIndex& index) const
{
    /* nothing */
}

/* ------------------------------------------------------------------------------------------------ */
void ColorItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
}

/* ------------------------------------------------------------------------------------------------ */
void ColorItemDelegate::paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QColor color = index.data().value<QColor>();
    if( p_painter ) { p_painter->fillRect(option.rect, QBrush(color)); }
}

/* ------------------------------------------------------------------------------------------------ */
QSize ColorItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return QSize(30, 30);
}

