#include "AspectItemDelegate.h"
#include "main/Definitions.h"
#include <QtGui>

/* ------------------------------------------------------------------------------------------------ */
AspectItemDelegate::AspectItemDelegate(QObject* p_parent) : QItemDelegate(p_parent)
{
}

/* ------------------------------------------------------------------------------------------------ */
QWidget *AspectItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QComboBox* p_box = new QComboBox(p_parent);
    p_box->addItem(Definitions::kTxtAspect_None);
    p_box->addItem(Definitions::kTxtAspect__4_3);
    p_box->addItem(Definitions::kTxtAspect_16_9);

    return p_box;
}

/* ------------------------------------------------------------------------------------------------ */
void AspectItemDelegate::setEditorData(QWidget* p_editor, const QModelIndex& index) const
{
    QString value = index.model()->data(index, Qt::EditRole).value<QString>();

    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( ! p_box ) { return; }
    p_box->setCurrentText(value);
    connect(p_box, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
}

/* ------------------------------------------------------------------------------------------------ */
void AspectItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( (! p_box) || (! p_model) ) { return; }

    p_model->setData(index, p_box->currentText(), Qt::EditRole);
}

/* ------------------------------------------------------------------------------------------------ */
void AspectItemDelegate::updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem&option, const QModelIndex& index ) const
{
    if( p_editor ) { p_editor->setGeometry(option.rect); }
}

/* ------------------------------------------------------------------------------------------------ */
void AspectItemDelegate::currentIndexChanged(int idx)
{
    QComboBox* p_box = static_cast<QComboBox*>(sender());
    if(! p_box) { return; }
    emit commitData(p_box);
}
