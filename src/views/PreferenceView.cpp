#include "PreferenceView.h"
#include "ui_PreferenceView.h"
#include "main/Definitions.h"

#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QIntValidator>
#include <QDir>
#include <QVariant>
#include <QObject>
#include <QMetaEnum>
#include <QDebug>
#include <QTextToSpeech>
#include <QCoreApplication>

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

/* ------------------------------------------------------------------------------------------------ */
PreferenceView::PreferenceView(QSettings* p_settings, QWidget *parent) : QWidget(parent)
, mp_ui(new Ui::PreferenceView)
, mp_settings( p_settings )
{
    mp_ui->setupUi(this);
    if( mp_ui->p_viewer_fps ) { mp_ui->p_viewer_fps->setValidator(new QIntValidator(15, 60, this)); }
    m_check_map[mp_ui->p_pilot_list_auto_refresh  ] = Definitions::kPilotListAutoRefresh;
    m_check_map[mp_ui->p_camera_find_and_activate ] = Definitions::kCameraFindAndActivate;
    m_check_map[mp_ui->p_auto_create_unknown_pilot] = Definitions::kAutoCreateUnknownPilot;
    m_check_map[mp_ui->p_speech_laptime           ] = Definitions::kSpeechLaptime;

    m_combo_map[mp_ui->p_viewer_fps]    = Definitions::kViewerFPS;
    m_combo_map[mp_ui->p_armarker_type] = Definitions::kARMarkerType;

    const QMetaObject& defs = Definitions::staticMetaObject;
    m_options = defs.enumerator( defs.indexOfEnumerator("Options") );

    QString path;

    path = mp_settings->value(m_options.valueToKey(Definitions::kPilotDir)).value<QString>();
    if( path.isEmpty() ) { path = QCoreApplication::applicationDirPath(); }
    mp_ui->p_pilotdir->setText(path);
    isExistingDirectory(path, false, mp_ui->p_pilotdir);

    path = mp_settings->value(m_options.valueToKey(Definitions::kEventDir)).value<QString>();
    if( path.isEmpty() ) { path = QCoreApplication::applicationDirPath(); }
    mp_ui->p_eventdir->setText(path);
    isExistingDirectory(path, false, mp_ui->p_eventdir);

    path = mp_settings->value(m_options.valueToKey(Definitions::kLayoutDir)).value<QString>();
    if( path.isEmpty() ) { path = QCoreApplication::applicationDirPath(); }
    mp_ui->p_layoutdir->setText(path);
    isExistingDirectory(path, false, mp_ui->p_layoutdir);

    for(QMap<QCheckBox*, int>::iterator it=m_check_map.begin(); it!=m_check_map.end(); ++it)
    {
        if( ! it.key() ) { continue; }
        bool check = mp_settings->value(m_options.valueToKey(it.value())).value<bool>();
        if( check ) { it.key()->setCheckState(Qt::Checked);   }
        else        { it.key()->setCheckState(Qt::Unchecked); }
    }

    for(QMap<QComboBox*, int>::iterator it=m_combo_map.begin(); it!=m_combo_map.end(); ++it)
    {
        if( ! it.key() ) { continue; }
        QString value = mp_settings->value(m_options.valueToKey(it.value())).value<QString>();
        if( value.isEmpty() ) { mp_settings->setValue(m_options.valueToKey(it.value()), it.key()->currentText()); }
        else                 { it.key()->setCurrentText(value); }
    }
}

/* ------------------------------------------------------------------------------------------------ */
PreferenceView::~PreferenceView(void)
{
    delete mp_ui;
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::openPilotDir(void)
{
    if( ! mp_settings ) { return; }
    if( ! mp_ui       ) { return; }
    if( ! mp_ui->p_pilotdir ) { return; }

    QString path = mp_ui->p_pilotdir->text();
    path = QFileDialog::getExistingDirectory(this,
                                            tr("Open Pilot Directory"),
                                            path,
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if( path.isEmpty() ) { return; }

    if( mp_ui->p_pilotdir ) { mp_ui->p_pilotdir->setText(path); }
    mp_settings->setValue(m_options.valueToKey(Definitions::kPilotDir), path);

    isExistingDirectory(path, true, mp_ui->p_pilotdir);

    emit settingChanged(Definitions::kPilotDir);
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::openEventDir(void)
{
    if( ! mp_settings ) { return; }
    if( ! mp_ui       ) { return; }
    if( ! mp_ui->p_eventdir ) { return; }

    QString path = mp_ui->p_eventdir->text();
    path = QFileDialog::getExistingDirectory(this,
                                            tr("Open Event Directory"),
                                            path,
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if( path.isEmpty() ) { return; }

    if( mp_ui->p_eventdir ) { mp_ui->p_eventdir->setText(path); }
    mp_settings->setValue(m_options.valueToKey(Definitions::kEventDir), path);

    isExistingDirectory(path, true, mp_ui->p_eventdir);

    emit settingChanged(Definitions::kEventDir);
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::openLayoutDir(void)
{
    if( ! mp_settings ) { return; }
    if( ! mp_ui       ) { return; }
    if( ! mp_ui->p_layoutdir ) { return; }

    QString path = mp_ui->p_layoutdir->text();
    path = QFileDialog::getExistingDirectory(this,
                                            tr("Open Layout Directory"),
                                            path,
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if( path.isEmpty() ) { return; }

    if( mp_ui->p_layoutdir ) { mp_ui->p_layoutdir->setText(path); }
    mp_settings->setValue(m_options.valueToKey(Definitions::kLayoutDir), path);

    isExistingDirectory(path, true, mp_ui->p_layoutdir);

    emit settingChanged(Definitions::kLayoutDir);
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::finishedPilotDirEditing(void)
{
    if( ! mp_settings ) { return; }
    if( ! mp_ui       ) { return; }
    if( ! mp_ui->p_pilotdir ) { return; }

    QString path = mp_ui->p_pilotdir->text();
    mp_settings->setValue(m_options.valueToKey(Definitions::kPilotDir), path);

    isExistingDirectory(path, ! mp_ui->p_pilotdir->hasFocus(), mp_ui->p_pilotdir);

    emit settingChanged(Definitions::kPilotDir);
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::finishedEventDirEditing(void)
{
    if( ! mp_settings ) { return; }
    if( ! mp_ui       ) { return; }
    if( ! mp_ui->p_eventdir ) { return; }

    QString path = mp_ui->p_eventdir->text();
    mp_settings->setValue(m_options.valueToKey(Definitions::kEventDir), path);

    isExistingDirectory(path, ! mp_ui->p_eventdir->hasFocus(), mp_ui->p_eventdir);

    emit settingChanged(Definitions::kEventDir);
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::finishedLayoutDirEditing(void)
{
    if( ! mp_settings ) { return; }
    if( ! mp_ui       ) { return; }
    if( ! mp_ui->p_layoutdir ) { return; }

    QString path = mp_ui->p_layoutdir->text();
    mp_settings->setValue(m_options.valueToKey(Definitions::kLayoutDir), path);

    isExistingDirectory(path, ! mp_ui->p_layoutdir->hasFocus(), mp_ui->p_layoutdir);

    emit settingChanged(Definitions::kLayoutDir);
}

/* ------------------------------------------------------------------------------------------------ */
bool PreferenceView::isExistingDirectory(QString path, bool mkdir, QLineEdit* p_target)
{
    QDir directory(path);

    if( p_target ) { p_target->setStyleSheet( "" ); }
    if( directory.exists() ) { return true; }

    if( ! mkdir ) { return false; }

    QMessageBox msg(this);
    msg.setText( path + tr(" is not exist. Do you want to create the directory?") );
    msg.setStandardButtons( QMessageBox::Yes | QMessageBox::No);
    int res = msg.exec();
    if( res != QMessageBox::Yes ) { return false; }

    directory.mkpath(path);
    if( ! directory.exists() ) { return false; }

    return true;

}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::stateChanged(int state)
{
    if( ! mp_settings ) { return; }

    bool check = (state != Qt::Unchecked);
    QCheckBox* p_sender = dynamic_cast<QCheckBox*>( sender() );
    if( ! m_check_map.contains(p_sender) ) { return; }
    mp_settings->setValue(m_options.valueToKey(m_check_map[p_sender]), check);

    emit settingChanged(m_check_map[p_sender]);
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::comboBoxChanged(QString number)
{
    if( ! mp_settings ) { return; }

    QComboBox* p_sender = dynamic_cast<QComboBox*>( sender() );
    if( ! m_combo_map.contains(p_sender) ) { return; }
    mp_settings->setValue(m_options.valueToKey(m_combo_map[p_sender]), number);
}

/* ------------------------------------------------------------------------------------------------ */
void PreferenceView::generateARMarkers(void)
{
    if( ! mp_settings ) { return; }
    if( ! mp_ui       ) { return; }
    if( ! mp_ui->p_armarker_type ) { return; }

    QString path = QFileDialog::getExistingDirectory(this,
                                            tr("ARMarker output Directory"),
                                            "",
                                            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if( path.isEmpty() ) { return; }

    QString text = mp_ui->p_armarker_type->currentText();
    cv::aruco::PREDEFINED_DICTIONARY_NAME type = cv::aruco::DICT_4X4_50;
    int count = 0;
    if( text == Definitions::kTxtARMarkerType_4X4_50  ) { type = cv::aruco::DICT_4X4_50;  count =  50; }
    if( text == Definitions::kTxtARMarkerType_4X4_100 ) { type = cv::aruco::DICT_4X4_100; count = 100; }
    if( text == Definitions::kTxtARMarkerType_4X4_250 ) { type = cv::aruco::DICT_4X4_250; count = 250; }

    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(type);
    cv::Mat image;

    for(int i=0; i<count; ++i)
    {
        QString fullpath = path + QString().sprintf("/marker_%03d.png", i);
        cv::aruco::drawMarker(dictionary, i, 200, image, 1);
        imwrite(fullpath.toStdString().c_str(), image);
    }
}

