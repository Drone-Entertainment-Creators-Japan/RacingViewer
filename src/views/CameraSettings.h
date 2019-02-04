#ifndef CAMERASETTINGS_H
#define CAMERASETTINGS_H

#include <QDialog>

namespace Ui { class CameraSettings; }

class QCamera;

class CameraSettings : public QDialog
{
    Q_OBJECT

public:
    explicit CameraSettings(QCamera* p_target, QWidget* p_parent = nullptr);
            ~CameraSettings(void);

public slots:
    void accepted(void);
private:
    Ui::CameraSettings* mp_ui;
    QCamera* mp_target;
};

#endif // CAMERASETTINGS_H
