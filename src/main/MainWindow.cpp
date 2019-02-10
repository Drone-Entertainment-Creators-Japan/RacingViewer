#include "MainWindow.h"

#include "views/PreferenceView.h"
#include "views/PilotListView.h"
#include "views/VideoAssignView.h"
#include "views/CourseView.h"
#include "views/LapView.h"
#include "views/AboutView.h"
#include "models/PilotListModel.h"
#include "models/VideoSourceModel.h"
#include "models/CourseModel.h"
#include "models/LapTimeCollection.h"
#include "videosources/IVideoSource.h"
#include "videosources/CameraVideoSource.h"

#include "Definitions.h"

#include <QtMultimedia/QCameraInfo>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QStyleOptionTab>
#include <QStandardItemModel>
#include <QApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QTimer>
#include <QDebug>

static const QString kCourseFile = "/Map1";

/* ------------------------------------------------------------------------------------------------ */
MainWindow::MainWindow(QWidget* p_parent) : QWidget(p_parent)
, mp_timer        ( nullptr )
, mp_pilot_list   ( nullptr )
, mp_video_sources( nullptr )
, mp_course       ( nullptr )
, mp_laps         ( nullptr )
, mp_settings     ( nullptr )
, mp_tab            ( nullptr )
, mp_preference_view( nullptr )
, mp_pilot_list_view( nullptr )
, mp_course_view    ( nullptr )
, mp_lap_view       ( nullptr )
{
    const QMetaObject& defs = Definitions::staticMetaObject;
    m_options = defs.enumerator( defs.indexOfEnumerator("Options") );

    mp_tab = new QTabWidget;
    mp_timer = new QTimer(this);

    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    mp_settings   = new QSettings(path + "/RacingViewer.ini", QSettings::IniFormat);
    mp_pilot_list = new PilotListModel(this);
    mp_video_sources = new VideoSourceModel(this);
    mp_course        = new CourseModel(this);
    mp_laps          = new LapTimeCollection(mp_course, this);
    connect(mp_pilot_list, SIGNAL(pilotDescChanged(QString, int, QVariant)), mp_video_sources, SLOT(pilotDescChanged(QString, int, QVariant)));
    connect(mp_pilot_list, SIGNAL(pilotDescChanged(QString, int, QVariant)), mp_laps         , SLOT(pilotDescChanged(QString, int, QVariant)));


    mp_preference_view = new PreferenceView(mp_settings);
    mp_pilot_list_view = new PilotListView(mp_pilot_list, mp_settings);
    mp_course_view     = new CourseView(mp_settings, mp_course);
    mp_lap_view        = new LapView(mp_settings, mp_video_sources, mp_pilot_list, mp_laps);
    QHBoxLayout* p_layout = new QHBoxLayout;
    if( p_layout ) { p_layout->addWidget(mp_tab); }
    setLayout(p_layout);

    if( mp_tab )
    {
        mp_tab->setTabPosition(QTabWidget::North);
        mp_tab->addTab( mp_preference_view, tr("Preference") );
        mp_tab->addTab( mp_pilot_list_view, tr("Pilots") );
        mp_tab->addTab( mp_course_view    , tr("Course") );
        mp_tab->addTab( mp_lap_view       , tr("Laptime") );
        mp_tab->addTab( new AboutView()   , tr("About") );
    }

    for(int i=0; i<Definitions::kOptionsCount; ++i)
    {
        settingChanged(i);
    }

    timeout();

    if( mp_timer )
    {
        mp_timer->setInterval(3000); /* 3sec */
        mp_timer->setSingleShot(false);
        connect(mp_timer, SIGNAL(timeout()), this, SLOT(timeout()));
        mp_timer->start();
    }

    connect(mp_preference_view, SIGNAL(settingChanged(int)), this, SLOT(settingChanged(int)));
    connect(mp_tab            , SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    addVideoAssignView();

    QSize size = mp_settings->value(m_options.valueToKey(Definitions::kMainWindowSize)).value<QSize>();
    if( size.width()  <= 0) { size.setWidth(600); }
    if( size.height() <= 0) { size.setHeight(400); }
    this->resize( size );
}

/* ------------------------------------------------------------------------------------------------ */
MainWindow::~MainWindow(void)
{
    delete mp_timer;

    delete mp_preference_view;
    delete mp_pilot_list_view;
    delete mp_tab;
    delete mp_video_sources;
    delete mp_pilot_list;
    delete mp_course;
    delete mp_settings;
}

/* ------------------------------------------------------------------------------------------------ */
void MainWindow::timeout(void)
{
    /* update cameras */
    bool activate = false;
    if( mp_settings ) { activate = mp_settings->value(m_options.valueToKey(Definitions::kCameraFindAndActivate)).toBool(); }

    if( ! mp_video_sources ) { return; }
    const QList<IVideoSource*>& sources = mp_video_sources->videoSources();
    QList<IVideoSource*> new_sources;

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &info, cameras)
    {
        int index = 0;
        for(; index<sources.count(); ++index)
        {
            if( ! sources[index] ) { continue; }

            CameraVideoSource* p_target = dynamic_cast<CameraVideoSource*>(sources[index]);
            bool available = (! p_target);
            if( p_target ) { available |= (p_target->deviceID() == info.deviceName()); }

            if( ! available ) { continue; }
            new_sources.push_back(sources[index]);
            break;
        }
        if( index < sources.count() ) { continue; }

        CameraVideoSource* p_camera = new CameraVideoSource(info.deviceName(), mp_pilot_list, mp_laps, this);

        if( ! p_camera ) { continue; }
        p_camera->setName( info.description() );
        p_camera->setActive(activate);
        new_sources.push_back(p_camera);

        if( ! mp_laps ) { continue; }
        connect(p_camera, SIGNAL(passedThrough(const QString&, int, qint64, double)), mp_laps, SLOT(passedThrough(const QString&, int, qint64, double)));
    }
    mp_video_sources->setVideoSources(new_sources);
}

/* ------------------------------------------------------------------------------------------------ */
void MainWindow::settingChanged(int index)
{
    if( ! mp_settings ) { return; }

    switch(index)
    {
    case Definitions::kPilotDir:
    {
        QString path = mp_settings->value(m_options.valueToKey(Definitions::kPilotDir) ).value<QString>();
        if( mp_pilot_list ) { mp_pilot_list->setPoolDirectory( path ); }
    } break;

    case Definitions::kEventDir:
    {
        QString path = mp_settings->value(m_options.valueToKey(Definitions::kEventDir) ).value<QString>();
        if( mp_course ) { mp_course->setCourseFilepath(path + "/" + kCourseFile + Definitions::kTxtCourseFileExtention); }
        if( mp_laps   ) { mp_laps->setPoolDirectory(path); }
    } break;

    case Definitions::kSpeechLaptime:
    {
        bool enable = mp_settings->value(m_options.valueToKey(Definitions::kSpeechLaptime) ).value<bool>();
        if( mp_laps ) { mp_laps->setSpeechEnable(enable); }
    } break;
    default: { /* nothing */ } break;
    }
}

/* ------------------------------------------------------------------------------------------------ */
void MainWindow::currentChanged(int index)
{
    /* update pilots */
    bool option = false;
    if( mp_settings ) { option = mp_settings->value(m_options.valueToKey(Definitions::kPilotListAutoRefresh)).value<bool>(); }
    if( option && mp_pilot_list ) { mp_pilot_list->refresh(); }
}

/* ------------------------------------------------------------------------------------------------ */
bool MainWindow::addVideoAssignView(void)
{
    if( ! mp_tab ) { return false; }
    VideoAssignView* p_view = new VideoAssignView(mp_settings, mp_video_sources, mp_pilot_list);
    mp_tab->insertTab(mp_tab->count()-1, p_view, tr("Screen 1"));
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void MainWindow::closeEvent(QCloseEvent* p_event)
{
    const QMetaObject& defs = Definitions::staticMetaObject;
    QMetaEnum options = defs.enumerator( defs.indexOfEnumerator("Options") );
    mp_settings->setValue(options.valueToKey(Definitions::kMainWindowSize), size());

    if( qApp ) { qApp->exit(); }
}


