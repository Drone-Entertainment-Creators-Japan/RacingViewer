#ifndef IMAGEITEMDELEGATE_H
#define IMAGEITEMDELEGATE_H

#include <QItemDelegate>

class ImageItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ImageItemDelegate(QObject* p_parent = nullptr);

    QWidget* createEditor(QWidget* p_parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const override;
    void drawDecoration(QPainter* p_painter, const QStyleOptionViewItem& option, const QRect& rect, const QPixmap& pixmap) const override;
};

#endif /* IMAGEITEMDELEGATE_H */
