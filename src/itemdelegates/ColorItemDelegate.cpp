#include "ColorItemDelegate.h"
#include "main/Definitions.h"
#include <QColorDialog>
#include <QImage>
#include <QPainter>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
ColorItemDelegate::ColorItemDelegate(QObject* p_parent) : QItemDelegate(p_parent)
{
}

/* ------------------------------------------------------------------------------------------------ */
QWidget* ColorItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QColorDialog* p_dialog = new QColorDialog(p_parent);

    return p_dialog;
}

/* ------------------------------------------------------------------------------------------------ */
void ColorItemDelegate::setEditorData(QWidget* p_editor, const QModelIndex& index) const
{
    QColorDialog* p_dialog = qobject_cast<QColorDialog*>(p_editor);
    if( p_dialog ) { p_dialog->setCurrentColor(index.data().value<QColor>() ); }
}

/* ------------------------------------------------------------------------------------------------ */
void ColorItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    QColorDialog* p_dialog = qobject_cast<QColorDialog*>(p_editor);
    if( ! p_dialog ) { return; }
    if( ! p_model ) { return; }

    if( p_dialog->result() == QDialog::Rejected ) { return; }
    QColor color = p_dialog->currentColor();
    p_model->setData(index, color, Qt::EditRole);
    p_model->setData(index, color, Qt::DecorationRole);
}

/* ------------------------------------------------------------------------------------------------ */
void ColorItemDelegate::drawDecoration(QPainter* p_painter, const QStyleOptionViewItem& option, const QRect& rect, const QPixmap& pixmap) const
{
    if( p_painter ) { p_painter->drawPixmap(option.rect, pixmap); }
}

/* ------------------------------------------------------------------------------------------------ */
void ColorItemDelegate::drawDisplay(QPainter* p_painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text  ) const
{
    /* nothing */
}
