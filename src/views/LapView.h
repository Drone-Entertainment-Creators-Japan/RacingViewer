#ifndef LAPVIEW_H
#define LAPVIEW_H

#include <QSplitter>
#include <QVariant>

class VideoSourceModel;
class PilotListModel;
class LapTimeCollection;
class LapTimeTableModel;
class LapTimeDetailModel;
class QListWidget;
class QTableView;
class QSettings;
class ViewWindow;
class QItemSelection;

class LapView : public QSplitter
{
    Q_OBJECT

public:
    explicit LapView(QSettings* p_settings, VideoSourceModel* p_source, PilotListModel* p_pilot_list, LapTimeCollection* p_laptimes, QWidget* p_parent=nullptr);
            ~LapView(void);

public slots:
    void pilotDescChanged(QString pilot, int descriptor_idx, QVariant value);

    void itemSelectionChanged(void);
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void rowsRemoved (const QModelIndex& parent, int first, int last);
    void rowsInserted(const QModelIndex& parent, int first, int last);
    void dataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

private:
    VideoSourceModel*   mp_video_model;
    PilotListModel*     mp_pilot_model;
    LapTimeTableModel*  mp_table_model;
    LapTimeDetailModel* mp_detail_model;
    LapTimeCollection*  mp_laptime;

    ViewWindow*   mp_preview;
    QListWidget*  mp_pilot_select;
    QTableView*   mp_lap_select;
    QTableView*   mp_lap_detail;
};

#endif /* LAPVIEW_H */
