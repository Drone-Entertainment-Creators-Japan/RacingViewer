#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>
#include <QtCore/QVector>
#include <QMetaEnum>

class VideoSourceModel;
class QSettings;
class QStandardItemModel;
class PreferenceView;
class PilotListView;
class PilotListModel;
class CourseView;
class CourseModel;
class LapView;
class LapTimeCollection;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* p_parent=nullptr);
            ~MainWindow(void) override;

private slots:
    void timeout(void);

    void settingChanged(int index);
    void currentChanged(int index);

private:
    bool addVideoAssignView(void);

    void closeEvent(QCloseEvent* p_event) override;
private:
    QMetaEnum         m_options;
    QTimer*           mp_timer;

    PilotListModel*    mp_pilot_list;
    VideoSourceModel*  mp_video_sources;
    CourseModel*       mp_course;
    LapTimeCollection* mp_laps;
    QSettings*         mp_settings;

    QTabWidget*       mp_tab;
    PreferenceView*   mp_preference_view;
    PilotListView*    mp_pilot_list_view;
    CourseView*       mp_course_view;
    LapView*          mp_lap_view;


};

#endif /* MAINWINDOW_H */
