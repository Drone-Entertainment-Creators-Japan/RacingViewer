#ifndef POINTITEMDELEGATE_H
#define POINTITEMDELEGATE_H

#include <QAbstractItemDelegate>

class PointItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit PointItemDelegate(QObject* p_parent=nullptr);

    void paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif /* POINTITEMDELEGATE_H */
