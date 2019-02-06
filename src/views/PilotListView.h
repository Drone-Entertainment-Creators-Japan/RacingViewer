#ifndef PILOTLISTVIEW_H
#define PILOTLISTVIEW_H

#include <QWidget>
#include <QMetaEnum>

namespace Ui { class PilotListView; }
class QSettings;
class PilotListModel;

class PilotListView : public QWidget
{
    Q_OBJECT

public:
    explicit PilotListView(PilotListModel* p_model, QSettings* p_setting, QWidget* parent=nullptr);
            ~PilotListView(void);

    bool eventFilter(QObject* p_watched, QEvent* p_event);

private:
    QMetaEnum       m_options;
    Ui::PilotListView* mp_ui;

    QSettings*      mp_settings;
    PilotListModel* mp_model;
};

#endif /* PILOTLISTVIEW_H */
