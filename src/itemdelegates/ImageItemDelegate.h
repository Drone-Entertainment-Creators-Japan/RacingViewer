#ifndef IMAGEITEMDELEGATE_H
#define IMAGEITEMDELEGATE_H

#include <QAbstractItemDelegate>

class ImageItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit ImageItemDelegate(QObject* p_parent = nullptr);

    QWidget* createEditor(QWidget* p_parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const override;
    void paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif /* IMAGEITEMDELEGATE_H */
