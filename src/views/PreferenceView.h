#ifndef PREFERENCEVIEW_H
#define PREFERENCEVIEW_H

#include <QWidget>
#include <QMetaEnum>
namespace Ui { class PreferenceView; }
class QSettings;
class QLineEdit;
class QCheckBox;
class QComboBox;

class PreferenceView : public QWidget
{
    Q_OBJECT

public:
    explicit PreferenceView(QSettings* p_settings, QWidget* p_parent = nullptr);
            ~PreferenceView(void);

signals:
    void settingChanged(int index);

public slots:
    void openPilotDir(void);
    void openEventDir(void);
    void openLayoutDir(void);

    void finishedPilotDirEditing(void);
    void finishedEventDirEditing(void);
    void finishedLayoutDirEditing(void);

    void stateChanged(int state);

    void comboBoxChanged(QString number);

    void generateARMarkers(void);
private:
    bool isExistingDirectory(QString path, bool mkdir, QLineEdit* target);

private:
    Ui::PreferenceView*   mp_ui;
    QSettings*            mp_settings;

    QMetaEnum             m_options;
    QMap<QCheckBox*, int> m_check_map;
    QMap<QComboBox*, int> m_combo_map;
};

#endif /* PREFERENCEVIEW_H */
