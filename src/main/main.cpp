
#include "MainWindow.h"
#include <QDebug>
#include <QCamera>
#include <QCameraInfo>
#include <QApplication>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();

    return app.exec();
}
