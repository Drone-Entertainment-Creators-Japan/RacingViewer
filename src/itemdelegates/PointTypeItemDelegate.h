#ifndef POINTTYPEITEMDELEGATE_H
#define POINTTYPEITEMDELEGATE_H


#include <QAbstractItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>

class PointTypeItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    PointTypeItemDelegate(QObject* p_parent=nullptr);

    QWidget* createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void setEditorData(QWidget* p_editor, const QModelIndex& index) const override;
    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model,const QModelIndex& index) const override;

    void updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    void paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

public slots:
    void currentIndexChanged(int idx);
};

#endif /* POINTTYPEITEMDELEGATE_H */
