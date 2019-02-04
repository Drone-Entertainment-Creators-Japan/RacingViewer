#ifndef VIDEOASSIGNVIEW_H
#define VIDEOASSIGNVIEW_H

#include <QtWidgets/QSplitter>

namespace Ui { class VideoAssignView; }
class QTableView;
class QSettings;
class VideoSourceModel;
class VideoAssignModel;
class PilotListModel;
class IVideoSource;
class ViewWindow;

class VideoAssignView : public QSplitter
{
    Q_OBJECT

public:
    explicit VideoAssignView(QSettings* p_settings, VideoSourceModel* p_source_model, PilotListModel* p_pilot_list, QWidget* p_parent=nullptr);
            ~VideoAssignView(void) override;

public slots:
    void vheaderClicked(int row);
    void vheaderDoubleClicked(int row);
    void hheaderClicked(int col);
    void activateChanged(void);
    void cutInChanged(IVideoSource* p_soure);
    void layoutChanged(int pattern);
    void assignChanged(int pattern);

    void mousePressed(IVideoSource* p_target);
    void doubleClicked(IVideoSource* p_target);

private:
    QVector<QRectF> readRects(const QString& filepath, bool* p_is_pixel_size);

private:
    QSettings*           mp_settings;
    VideoAssignModel*    mp_model;
    int                  m_selected_pattern;
    QTableView*          mp_table;
    ViewWindow*          mp_preview;
    ViewWindow*          mp_view;
    QTimer*              mp_timer;
};

#endif /* VIDEOASSIGNVIEW_H */
