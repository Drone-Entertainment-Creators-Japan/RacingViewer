#include "LapView.h"

#include "models/PilotListModel.h"
#include "models/LapTimeRecord.h"
#include "models/LapTimeCollection.h"
#include "models/LapTimeTableModel.h"
#include "models/LapTimeDetailModel.h"
#include "models/VideoSourceModel.h"
#include "videosources/IVideoSource.h"
#include "ViewWindow.h"
#include "main/Definitions.h"

#include <QtWidgets/QListWidget>
#include <QtWidgets/QTableView>
#include <QItemSelectionModel>

/* ------------------------------------------------------------------------------------------------ */
LapView::LapView(QSettings* p_settings, VideoSourceModel* p_source, PilotListModel* p_pilot_list, LapTimeCollection* p_laptimes, QWidget* p_parent) : QSplitter(p_parent)
, mp_video_model ( p_source )
, mp_pilot_model ( p_pilot_list )
, mp_table_model ( nullptr      )
, mp_detail_model( nullptr      )
, mp_laptime     ( p_laptimes   )
, mp_preview     ( nullptr )
, mp_pilot_select( nullptr )
, mp_lap_select  ( nullptr )
, mp_lap_detail  ( nullptr )
{
    connect(mp_video_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int))                               , this, SLOT(rowsRemoved(const QModelIndex&, int, int)));
    connect(mp_video_model, SIGNAL(rowsInserted(const QModelIndex&, int, int))                              , this, SLOT(rowsInserted(const QModelIndex&, int, int)));
    connect(mp_video_model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)), this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));


    setOrientation(Qt::Vertical);
    mp_preview = new ViewWindow(p_settings);

    mp_table_model  = new LapTimeTableModel(this);
    mp_detail_model = new LapTimeDetailModel(this);

    mp_pilot_select  = new QListWidget;
    mp_lap_select    = new QTableView;
    mp_lap_detail    = new QTableView;

    mp_pilot_select->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(mp_pilot_select, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
    mp_lap_select->setModel(mp_table_model);

    mp_lap_select->setSelectionMode(QAbstractItemView::MultiSelection);
    QItemSelectionModel* p_selection = mp_lap_select->selectionModel();
    connect(p_selection, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));

    mp_lap_detail->setModel( mp_detail_model );

    addWidget(mp_preview);
    QSplitter* p_splitter = new QSplitter();
    p_splitter->setOrientation(Qt::Horizontal);
    p_splitter->addWidget(mp_pilot_select);
    p_splitter->addWidget(mp_lap_select);
    p_splitter->addWidget(mp_lap_detail);
    addWidget(p_splitter);

    mp_pilot_select->setMinimumWidth(70);
    mp_preview->setMinimumHeight(70);
    if( mp_pilot_model )
    {
        for(int i=0; i<mp_pilot_model->rowCount(); ++i)
        {
            QString pilot = mp_pilot_model->valueOf(i, Definitions::kPilotName).toString();
            if( pilot.isEmpty() ) { continue; }
            mp_pilot_select->addItem( pilot );
        }
        connect(mp_pilot_model, SIGNAL(pilotDescChanged(QString, int, QVariant)), this, SLOT(pilotDescChanged(QString, int, QVariant)));
    }
}

/* ------------------------------------------------------------------------------------------------ */
LapView::~LapView()
{
    delete mp_detail_model;
    delete mp_table_model;
    delete mp_pilot_select;
}

/* ------------------------------------------------------------------------------------------------ */
void LapView::pilotDescChanged(QString pilot, int descriptor_idx, QVariant value)
{
    if( ! mp_pilot_select ) { return; }
    if( descriptor_idx != Definitions::kPilotName ) { return; }

    QString new_value = value.toString();

    if( (pilot.isEmpty()) && (! new_value.isEmpty() ) ) /* add */
    {
        QList<QListWidgetItem*> items = mp_pilot_select->findItems(pilot, Qt::MatchExactly);
        if( ! items.isEmpty() ) { return; }
        mp_pilot_select->addItem(new_value);
    }

    if( (! pilot.isEmpty()) && (new_value.isEmpty() ) ) /* remove */
    {
        QList<QListWidgetItem*> items = mp_pilot_select->findItems(pilot, Qt::MatchExactly);
        for(int i=0; i<items.count(); ++i) { delete mp_pilot_select->takeItem( mp_pilot_select->row(items[i]) ); }
    }

    if( (! pilot.isEmpty()) && (! new_value.isEmpty() ) ) /* change */
    {
        QList<QListWidgetItem*> items = mp_pilot_select->findItems(pilot, Qt::MatchExactly);
        for(int i=0; i<items.count(); ++i) { items[i]->setData(Qt::EditRole, new_value); }
    }

    itemSelectionChanged();
}

/* ------------------------------------------------------------------------------------------------ */
void LapView::itemSelectionChanged(void)
{
    if( ! mp_pilot_select  ) { return; }
    if( ! mp_table_model ) { return; }
    if( ! mp_laptime       ) { return; }

    QList<IVideoSource*> sources ;
    if( mp_video_model ) { sources = mp_video_model->videoSources(); }

    QVector<IVideoSource*> source_list;
    QList<QListWidgetItem*> selected = mp_pilot_select->selectedItems();
    QStringList pilots;
    int camera_count = 0;
    for(int i=0; i<selected.count(); ++i)
    {
        if( ! selected[i] ) { continue; }
        QString pilot = selected[i]->data(Qt::DisplayRole).toString();
        if( pilot.isEmpty() ) { continue; }

        int j=0;
        for(; j<sources.count(); ++j)
        {
            if( ! sources[j] ) { continue;}
            if( pilot != sources[j]->value(Definitions::kPilotAssign).toString() ) { continue; }
            source_list.push_back(sources[j]);
        }

        if( j == sources.count() )
        {
            pilots.push_back(pilot);
        } else
        {
            pilot.insert(camera_count, pilot);
            camera_count += 1;
        }

    }
    QList<LapTimeRecord*> laps = mp_laptime->list(pilots);

    mp_table_model->setLapTimeList(laps);
    if( mp_preview ) { mp_preview->setVideoSources(source_list); }
}

/* ------------------------------------------------------------------------------------------------ */
void LapView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if( ! mp_laptime      ) { return; }
    if( ! mp_table_model  ) { return; }
    if( ! mp_detail_model ) { return; }

    QList<LapTimeRecord*> laps = mp_laptime->list();
    QItemSelectionModel* p_selection = mp_lap_select->selectionModel();
    if( ! p_selection ) { return; }
    QModelIndexList indexes = p_selection->selectedIndexes();
    QList<LapTimeDetailModel::Item> detail_list;
    for(int i=0; i<indexes.count(); ++i)
    {
        int col    = indexes[i].column();
        int lap_id = indexes[i].row();

        if( lap_id < 0 ) { continue; }
        QString pilot = mp_table_model->headerData(col, Qt::Horizontal).toString();
        if( pilot.isEmpty() ) { continue; }

        for(int j=0; j<laps.count(); ++j)
        {
            if( ! laps[j] ) { continue; }
            if( laps[j]->name() != pilot ) { continue; }

            if( lap_id == 0 ) { laps[j]->getBestTime(LapTimeRecord::kSection_LapTime, &lap_id); }
            else              { lap_id -= 1; }
            LapTimeDetailModel::Item item;
            item.lap_id = lap_id;
            item.p_record = laps[j];
            detail_list.push_back(item);
            break;
        }
    }

    mp_detail_model->setItemList(detail_list);
}

/* ------------------------------------------------------------------------------------------------ */
void LapView::rowsRemoved (const QModelIndex& parent, int first, int last)
{
    itemSelectionChanged();
}

/* ------------------------------------------------------------------------------------------------ */
void LapView::rowsInserted(const QModelIndex& parent, int first, int last)
{
    itemSelectionChanged();
}

/* ------------------------------------------------------------------------------------------------ */
void LapView::dataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    itemSelectionChanged();
}

