#ifndef COLORITEMDELEGATE_H
#define COLORITEMDELEGATE_H

#include <QItemDelegate>

class ColorItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ColorItemDelegate(QObject* p_parent = nullptr);

    QWidget* createEditor(QWidget* p_parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget* p_editor, const QModelIndex& index) const override;
    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const override;

    void drawDecoration(QPainter* p_painter, const QStyleOptionViewItem& option, const QRect& rect, const QPixmap& pixmap) const override;
    void drawDisplay   (QPainter* p_painter, const QStyleOptionViewItem& option, const QRect& rect, const QString& text  ) const override;
};

#endif /* COLORITEMDELEGATE_H */
