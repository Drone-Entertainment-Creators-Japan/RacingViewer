#include "PointTypeItemDelegate.h"
#include "main/Definitions.h"
#include <QtGui>
#include <QRect>
#include <QPainter>


/* ------------------------------------------------------------------------------------------------ */
PointTypeItemDelegate::PointTypeItemDelegate(QObject* p_parent) : QAbstractItemDelegate(p_parent)
{
}

/* ------------------------------------------------------------------------------------------------ */
QWidget *PointTypeItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QComboBox* p_box = new QComboBox(p_parent);
    p_box->addItem(Definitions::kTxtPoint_None);
    p_box->addItem(Definitions::kTxtPoint_StartAndGoal);
    p_box->addItem(Definitions::kTxtPoint_Start);
    p_box->addItem(Definitions::kTxtPoint_Goal);
    p_box->addItem(Definitions::kTxtPoint_CheckPoint);
    p_box->addItem(Definitions::kTxtPoint_Through);

    return p_box;
}

/* ------------------------------------------------------------------------------------------------ */
void PointTypeItemDelegate::setEditorData(QWidget* p_editor, const QModelIndex& index) const
{
    int type = index.model()->data(index, Qt::EditRole).toInt();

    QString type_text = Definitions::toTextPointType(type);

    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( p_box ) { p_box->setCurrentText(type_text); }
}

/* ------------------------------------------------------------------------------------------------ */
void PointTypeItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( (! p_box) || (! p_model) ) { return; }

    int type = Definitions::toIntPointType( p_box->currentText() );
    p_model->setData(index, type, Qt::EditRole);
}

/* ------------------------------------------------------------------------------------------------ */
void PointTypeItemDelegate::updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem&option, const QModelIndex& index ) const
{
    if( p_editor ) { p_editor->setGeometry(option.rect); }
}

/* ------------------------------------------------------------------------------------------------ */
void PointTypeItemDelegate::paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if( ! p_painter ) { return; }
    int type = index.data().value<int>();

    QString type_text = Definitions::toTextPointType(type);
    QRect rect = option.rect;
    p_painter->drawText(rect, Qt::AlignCenter, type_text, &rect);
}

/* ------------------------------------------------------------------------------------------------ */
QSize PointTypeItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QFont font;
    QFontMetrics metrics(font);
    int type = index.data().value<int>();

    QString type_text = Definitions::toTextPointType(type);

    return metrics.boundingRect(type_text).size();
}

