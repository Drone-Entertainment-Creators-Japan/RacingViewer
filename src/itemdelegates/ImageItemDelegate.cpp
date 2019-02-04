#include "ImageItemDelegate.h"
#include "main/Definitions.h"
#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
ImageItemDelegate::ImageItemDelegate(QObject* p_parent) : QItemDelegate(p_parent)
{
}

/* ------------------------------------------------------------------------------------------------ */
QWidget* ImageItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QFileDialog* p_dialog = new QFileDialog(p_parent);
    p_dialog->setNameFilter("*.*");
    p_dialog->setAcceptMode(QFileDialog::AcceptOpen);
//    p_dialog->setOption(QFileDialog::DontUseNativeDialog);

    return p_dialog;
}

/* ------------------------------------------------------------------------------------------------ */
void ImageItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    QFileDialog* p_dialog = qobject_cast<QFileDialog*>(p_editor);
    if( ! p_dialog ) { return; }
    if( ! p_model ) { return; }

    if( p_dialog->result() == QDialog::Rejected ) { return; }
    QStringList file_list = p_dialog->selectedFiles();
    if( file_list.empty() ) { return; }

    QImage image(file_list[0]);
    if( image.isNull() ) { return; }
    QRect source_rect = image.rect();
    if( source_rect.width() > source_rect.height() ) { source_rect.setHeight(source_rect.width()); }
    else                                             { source_rect.setWidth (source_rect.height()); }
    QImage pilot_image(Definitions::kPilotImageSize, Definitions::kPilotImageSize, QImage::Format_ARGB32);
    pilot_image.fill(QColor(0, 0, 0, 0));
    QPainter pilot_image_painter(&pilot_image);
    pilot_image_painter.drawImage(pilot_image.rect(), image, source_rect);

    p_model->setData(index, pilot_image, Qt::EditRole);
    p_model->setData(index, pilot_image, Qt::DecorationRole);
}

/* ------------------------------------------------------------------------------------------------ */
void ImageItemDelegate::drawDecoration(QPainter* p_painter, const QStyleOptionViewItem& option, const QRect& rect, const QPixmap& pixmap) const
{
    if( pixmap.width()  == 0 ) { return; }
    if( pixmap.height() == 0 ) { return; }
    double   rate = option.rect.width()  / static_cast<double>(pixmap.width());
    double v_rate = option.rect.height() / static_cast<double>(pixmap.height());
    if( rate > v_rate ) { rate = v_rate; }

    QRect inner_rect(0, 0, static_cast<int>(pixmap.width()*rate), static_cast<int>(pixmap.height()*rate));
    inner_rect.moveCenter( option.rect.center() );

    if( p_painter ) { p_painter->drawPixmap(inner_rect, pixmap); }
}

