#include "PilotListView.h"
#include "ui_PilotListView.h"

#include "models/PilotListModel.h"
#include "itemdelegates/ImageItemDelegate.h"
#include "itemdelegates/ColorItemDelegate.h"
#include "itemdelegates/AspectItemDelegate.h"
#include "main/Definitions.h"

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
    mp_model->setData(index, QVariant(), Qt::DecorationRole);

    return false;
}

