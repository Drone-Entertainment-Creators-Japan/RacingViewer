#include "PilotListView.h"
#include "ui_PilotListView.h"

#include "models/PilotListModel.h"
#include "itemdelegates/ImageItemDelegate.h"
#include "itemdelegates/ColorItemDelegate.h"
#include "itemdelegates/AspectItemDelegate.h"
#include "main/Definitions.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QPainter>
#include <QModelIndex>
#include <QSettings>
#include <QKeyEvent>
#include <QEvent>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
PilotListView::PilotListView(PilotListModel* p_model, QSettings* p_setting, QWidget* p_parent) : QWidget(p_parent)
, mp_ui      ( new Ui::PilotListView )
, mp_settings( p_setting )
, mp_model   ( p_model   )
{
    const QMetaObject& defs = Definitions::staticMetaObject;
    m_options = defs.enumerator( defs.indexOfEnumerator("Options") );

    mp_ui->setupUi(this);
    mp_ui->p_pilot_list->setModel(mp_model);
    mp_ui->p_pilot_list->setItemDelegateForColumn(Definitions::kPilotImage, new ImageItemDelegate(this));
    mp_ui->p_pilot_list->setItemDelegateForColumn(Definitions::kPilotColor, new ColorItemDelegate(this));
    mp_ui->p_pilot_list->setItemDelegateForColumn(Definitions::kCameraAspectRatio, new AspectItemDelegate(this));
    mp_ui->p_pilot_list->installEventFilter(this);

    connect(mp_ui->p_pilot_list, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked(const QModelIndex&)));
}

/* ------------------------------------------------------------------------------------------------ */
PilotListView::~PilotListView(void)
{
    delete mp_ui;
}

/* ------------------------------------------------------------------------------------------------ */
bool PilotListView::eventFilter(QObject* p_watched, QEvent* p_event)
{
    if( ! mp_ui ) { return false; }
    if( ! mp_ui->p_pilot_list ) { return false; }
    if( ! mp_model ) { return false; }

    QKeyEvent* p_key_event = static_cast<QKeyEvent*>(p_event);
    if( ! p_key_event ) { return false; }
    if( p_key_event->type() != QEvent::KeyPress ) { return false; }

    bool remove_key_pressed = false;
    remove_key_pressed |= (p_key_event->key() == Qt::Key_Delete);
    remove_key_pressed |= (p_key_event->key() == Qt::Key_Backspace);
    if( ! remove_key_pressed ) { return false; }

    QModelIndex index = mp_ui->p_pilot_list->currentIndex();
    if( index.column() != Definitions::kPilotImage ) { return false; }

    mp_model->setData(index, QVariant(), Qt::EditRole);

    return false;
}

/* ------------------------------------------------------------------------------------------------ */
void PilotListView::doubleClicked(const QModelIndex& index)
{
    if( ! mp_model ) { return; }
    int row = index.row();
    int col = index.column();

    if( col == Definitions::kPilotImage )
    {
        QFileDialog dialog(this);
        dialog.setNameFilter("*.*");
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        dialog.exec();

        if( dialog.result() == QDialog::Rejected ) { return; }
        QStringList file_list = dialog.selectedFiles();
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

        mp_model->setData(index, pilot_image, Qt::EditRole);
        mp_model->setData(index, pilot_image, Qt::DecorationRole);
    }

    if( col == Definitions::kPilotColor )
    {
        QColorDialog dialog(this);
        dialog.setCurrentColor(index.data().value<QColor>() );
        dialog.exec();

        if( dialog.result() == QDialog::Rejected ) { return; }
        QColor color = dialog.currentColor();
        mp_model->setData(index, color, Qt::EditRole);
    }
}


