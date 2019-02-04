#include "PointItemDelegate.h"
#include "main/Definitions.h"

#include <QFontMetrics>
#include <QString>
#include <QVariant>
#include <QPointF>
#include <QPainter>

/* ------------------------------------------------------------------------------------------------ */
PointItemDelegate::PointItemDelegate(QObject* p_parent) : QAbstractItemDelegate(p_parent)
{
}

/* ------------------------------------------------------------------------------------------------ */
void PointItemDelegate::paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if( ! p_painter ) { return; }
    QPointF point = index.data().value<QPointF>();
    QString text = Definitions::fromPointF(point);
    QRect rect = p_painter->boundingRect(option.rect, Qt::AlignCenter, text);
    p_painter->drawText(rect, text);
}

/* ------------------------------------------------------------------------------------------------ */
QSize PointItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QFont font;
    QFontMetrics metrics(font);

    QPointF point = index.data().value<QPointF>();
    QString text = Definitions::fromPointF(point);
    return metrics.boundingRect(text).size();
}
