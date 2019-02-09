
#include "MainWindow.h"
#include <QDebug>
#include <QCamera>
#include <QCameraInfo>
#include <QApplication>
#include <QTime>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();

    QTime time(0,0,0,0);
    int msec = time.msecsTo(QTime::fromMSecsSinceStartOfDay(1000));


    return app.exec();
}
