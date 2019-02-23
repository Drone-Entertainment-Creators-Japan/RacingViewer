#include "VideoAssignView.h"

#include "models/VideoAssignModel.h"
#include "models/PilotListModel.h"
#include "videosources/IVideoSource.h"
#include "videosources/CameraVideoSource.h"
#include "itemdelegates/PilotItemDelegate.h"
#include "itemdelegates/FileItemDelegate.h"
#include "main/Definitions.h"
#include "ViewWindow.h"
#include "CameraSettings.h"

#include <QtCore/QTimer>
#include <QtCore/QVector>
#include <QtCore/QList>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QItemSelectionModel>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHeaderView>
#include <stdio.h>

VideoAssignView::VideoAssignView(QSettings* p_settings, VideoSourceModel* p_source_model, PilotListModel* p_pilot_list, QWidget* p_parent) : QSplitter(p_parent)
, mp_settings( p_settings )
, mp_model   ( nullptr )
, m_selected_pattern( 0 )
, mp_table  ( nullptr )
, mp_preview( nullptr )
, mp_view   ( nullptr )
, mp_timer  ( nullptr )
{
    setOrientation(Qt::Vertical);

    mp_view = new ViewWindow(p_settings);
    mp_view->show();

    mp_model = new VideoAssignModel(p_source_model);
    connect(mp_model, SIGNAL(activateChanged())          , this, SLOT(activateChanged()));
    connect(mp_model, SIGNAL(cutInChanged(IVideoSource*)), this, SLOT(cutInChanged(IVideoSource*)));
    connect(mp_model, SIGNAL(layoutChanged(int))         , this, SLOT(layoutChanged(int)));
    connect(mp_model, SIGNAL(assignChanged(int))         , this, SLOT(assignChanged(int)));

    mp_preview = new ViewWindow(p_settings);
    mp_preview->setMinimumHeight(100);
    mp_preview->setViewLoad(true);
    connect(mp_preview, SIGNAL(mousePressed(IVideoSource*)),  this, SLOT(mousePressed(IVideoSource*)));
    connect(mp_preview, SIGNAL(doubleClicked(IVideoSource*)), this, SLOT(doubleClicked(IVideoSource*)));

    addWidget(mp_preview);
    mp_table = new QTableView;
    addWidget(mp_table);
    mp_table->setModel(mp_model);
//    mp_table->setSortingEnabled(true);
    connect(mp_table->verticalHeader(),   SIGNAL(sectionClicked(int))      , this, SLOT(vheaderClicked(int)));
    connect(mp_table->verticalHeader(),   SIGNAL(sectionDoubleClicked(int)), this, SLOT(vheaderDoubleClicked(int)));
    connect(mp_table->horizontalHeader(), SIGNAL(sectionClicked(int))      , this, SLOT(hheaderClicked(int)));

    assignChanged( m_selected_pattern );
    activateChanged();

    if( mp_table->horizontalHeader() ) { mp_table->horizontalHeader()->setDefaultSectionSize(50); }
    mp_table->resizeColumnsToContents();
    mp_table->setItemDelegateForColumn(Definitions::kPilotAssign, new PilotItemDelegate(mp_settings, p_pilot_list));
    mp_table->setItemDelegateForRow(VideoAssignModel::kLayoutType, new FileItemDelegate(mp_settings, Definitions::kTxtLayoutFileExtention));
    mp_table->setItemDelegateForRow(VideoAssignModel::kCutInType,  new FileItemDelegate(mp_settings, Definitions::kTxtCutInFileExtention));
    mp_table->setColumnWidth(Definitions::kDeviceID   ,  60);
    mp_table->setColumnWidth(Definitions::kPilotAssign, 100);
    mp_table->setColumnWidth(Definitions::kVideoSourceDescriptorCount+VideoAssignModel::kColHeader, 50);
}

/* ------------------------------------------------------------------------------------------------ */
VideoAssignView::~VideoAssignView(void)
{
    if( mp_timer ) { mp_timer->stop(); }
    delete mp_table;
    delete mp_preview;
    delete mp_model;
    delete mp_view;
    delete mp_timer;
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::vheaderClicked(int row)
{
    if( ! mp_model ) { return; }
    if( row < 0 ) { return; }
    if( row >= mp_model->rowCount() ) { return; }

    IVideoSource* p_source = mp_model->videoSource(row-VideoAssignModel::kRowHeader);

    mp_preview->selectVideoSource(p_source);
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::vheaderDoubleClicked(int row)
{
    if( ! mp_model ) { return; }
    if( row < 0 ) { return; }
    if( row >= mp_model->rowCount() ) { return; }

    CameraVideoSource* p_source = dynamic_cast<CameraVideoSource*>(mp_model->videoSource(row-VideoAssignModel::kRowHeader));
    if( ! p_source ) { return; }
    QCamera* p_camera = p_source->camera();
    if( ! p_camera ) { return; }
    CameraSettings dialog(p_camera);
    dialog.exec();
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::hheaderClicked(int col)
{
    int pattern = mp_model->colToPattern(col);
    if( pattern < 0 ) { return;}
    if( pattern == m_selected_pattern ) { return; }

    m_selected_pattern = pattern;
    layoutChanged(pattern);
    assignChanged(pattern);
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::activateChanged(void)
{
    if( ! mp_preview ) { return; }
    if( ! mp_model   ) { return; }

    QVector<IVideoSource*> list;
    for(int i=0; i<=mp_model->videoSourceCount(); ++i)
    {
        IVideoSource* p_source = mp_model->videoSource(i);
        if( ! p_source ) { continue; }
        if( ! p_source->value(Definitions::kActiveState).toBool() ) { continue; }
        list.push_back(p_source);
    }
    mp_preview->setVideoSources(list);
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::cutInChanged(IVideoSource* p_source)
{
    if( ! mp_view ) { return; }
    mp_view->setCutIn(p_source);
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::layoutChanged(int pattern)
{
    if( ! mp_view  ) { return; }
    if( ! mp_table ) { return; }
    if( ! mp_model ) { return; }
    if( pattern != m_selected_pattern ) { return; }

    const QMetaObject& defs = Definitions::staticMetaObject;
    QMetaEnum options = defs.enumerator( defs.indexOfEnumerator("Options") );
    QString path = mp_settings->value(options.valueToKey(Definitions::kLayoutDir)).value<QString>();
    QString type;
    bool    is_pixel_size = false;

    QVector<QRectF> layout;
    type = mp_model->layoutType(m_selected_pattern);
    if( type != Definitions::kTxtLayout_Auto ) { layout = readRects(path + "/" + type + Definitions::kTxtLayoutFileExtention, &is_pixel_size); }
    mp_view->setFixedLayout(layout, is_pixel_size);

    layout.clear();
    type = mp_model->cutinType(m_selected_pattern);
    if( type != Definitions::kTxtLayout_Auto ) { layout = readRects(path + "/" + type + Definitions::kTxtCutInFileExtention, &is_pixel_size); }
    mp_view->setFixedCutIn(layout, is_pixel_size);
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::assignChanged(int pattern)
{
    if( ! mp_view  ) { return; }
    if( ! mp_table ) { return; }
    if( ! mp_model ) { return; }
    if( pattern != m_selected_pattern ) { return; }

    QVector<IVideoSource*> list;
    for(int i=0; i<=mp_model->videoSourceCount(); ++i)
    {
        int number = mp_model->assign(pattern, i) - 1;
        if( number < 0 ) { continue; }
        if( number >= list.count() ) { list.resize(number+1); }
        list[number] = mp_model->videoSource(i);
    }

    mp_view->setVideoSources(list);
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::mousePressed(IVideoSource* p_target)
{
    if( ! p_target ) { return; }
    if( ! mp_table ) { return; }
    if( ! mp_model ) { return; }

    QVector<IVideoSource*> list;
    for(int i=0; i<=mp_model->videoSourceCount(); ++i)
    {
        if( mp_model->videoSource(i) != p_target ) { continue; }
        if( mp_table->horizontalHeader() ) { mp_table->verticalHeader(); }

        QItemSelectionModel* p_selection = mp_table->selectionModel();
        if( ! p_selection ) { continue; }
        p_selection->clear();
        p_selection->select(mp_model->index(i+VideoAssignModel::kRowHeader, 1), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        if( mp_preview ) { mp_preview->selectVideoSource(p_target); }
        break;
    }
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignView::doubleClicked(IVideoSource* p_target)
{
    if( ! p_target ) { return; }
    if( ! mp_table ) { return; }
    if( ! mp_model ) { return; }

    QVector<IVideoSource*> list;
    for(int i=0; i<=mp_model->videoSourceCount(); ++i)
    {
        if( mp_model->videoSource(i) != p_target ) { continue; }
        if( mp_table->horizontalHeader() ) { mp_table->verticalHeader(); }

        mp_model->setCutin(i, ! mp_model->cutin(i));
        break;
    }
}

/* ------------------------------------------------------------------------------------------------ */
QVector<QRectF> VideoAssignView::readRects(const QString& filepath, bool* p_is_pixel_size)
{
    QVector<QRectF> result;

    QFile file(filepath);

    file.open(QIODevice::ReadOnly);
    bool is_pixel_size = (QString(file.readLine()).toUInt() != 0);
    while( ! file.atEnd() )
    {
        QByteArray line = file.readLine();
        double x      = 0;
        double y      = 0;
        double width  = 0;
        double height = 0;
        int cnt = sscanf(line.data(), "%*[^0123456789]%lf%*[^0123456789]%lf%*[^0123456789]%lf%*[^0123456789]%lf", &x, &y, &width, &height);
        if( cnt < 4 ) { continue; }

        result.push_back( QRectF(x, y, width, height));
    }
    if( p_is_pixel_size ) { *p_is_pixel_size = is_pixel_size; }
    return result;
}
