#include "CheckboxItemDelegate.h"

/* ------------------------------------------------------------------------------------------------ */
CheckboxItemDelegate::CheckboxItemDelegate(QObject* p_parent) : QStyledItemDelegate(p_parent)
{

}

/* ------------------------------------------------------------------------------------------------ */
QWidget* CheckboxItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return new QCheckBox(p_parent);
}

/* ------------------------------------------------------------------------------------------------ */
void CheckboxItemDelegate::setEditorData(QWidget* p_editor, const QModelIndex& index) const
{
    QCheckBox* p_box = qobject_cast<QCheckBox*>(p_editor);
    if( p_box ) { p_box->setChecked( index.data().toBool() ); }
}

/* ------------------------------------------------------------------------------------------------ */
void CheckboxItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    QCheckBox* p_box = qobject_cast<QCheckBox*>(p_editor);
    if( ! p_box ) { return; }
    bool value = (p_box->checkState() == Qt::Checked);
    p_model->setData(index, value, Qt::EditRole);
}

/* ------------------------------------------------------------------------------------------------ */
void CheckboxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( ! QApplication::style() ) { return; }
    bool data = index.model()->data(index, Qt::DisplayRole).toBool();

    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() + option.rect.width()/2 - checkbox_rect.width()/2);
    //checked or not checked
    if(data) { checkboxstyle.state = QStyle::State_On|QStyle::State_Enabled; }
    else     { checkboxstyle.state = QStyle::State_Off|QStyle::State_Enabled; }

    QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxstyle, painter);
}

/* ------------------------------------------------------------------------------------------------ */
void CheckboxItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( ! QApplication::style() ) { return; }

    QStyleOptionButton checkboxstyle;
    QRect checkbox_rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxstyle);

    checkboxstyle.rect = option.rect;
    checkboxstyle.rect.setLeft(option.rect.x() + option.rect.width()/2 - checkbox_rect.width()/2);

    editor->setGeometry(checkboxstyle.rect);
}
