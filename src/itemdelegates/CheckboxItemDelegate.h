#ifndef CHECKBOXITEMDELEGATE_H
#define CHECKBOXITEMDELEGATE_H

#include <QApplication>
#include <QStyledItemDelegate>
#include <QCheckBox>
#include <QPainter>

class CheckboxItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CheckboxItemDelegate(QObject* p_parent=nullptr);

    QWidget *createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* p_editor, const QModelIndex& index) const override;
    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const override;
    void paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

};

#endif /* CHECKBOXITEMDELEGATE_H */
