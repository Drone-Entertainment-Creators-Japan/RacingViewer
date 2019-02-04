#ifndef ASPECTITEMDELEGATE_H
#define ASPECTITEMDELEGATE_H


#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>

class AspectItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    AspectItemDelegate(QObject* p_parent=nullptr);

    QWidget *createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void setEditorData(QWidget* p_editor, const QModelIndex& index) const;
    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model,const QModelIndex& index) const;

    void updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif /* ASPECTITEMDELEGATE_H */
