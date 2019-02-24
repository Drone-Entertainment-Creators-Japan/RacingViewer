#-------------------------------------------------
#
# Project created by QtCreator 2018-12-15T00:17:39
#
#-------------------------------------------------

QT += core gui multimedia texttospeech

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RacingViewer
TEMPLATE = app

#TRANSLATIONS += translation_ja.ts

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

OPENCV = C:/Projects/RacingViewer/lib/opencv-4.0.1/build

SOURCES += \
    ./main/main.cpp \
    ./main/MainWindow.cpp \
    ./main/Definitions.cpp \
    ./views/VideoAssignView.cpp \
    ./views/ViewWindow.cpp \
    ./views/CameraSettings.cpp \
    ./views/PreferenceView.cpp \
    ./views/PilotListView.cpp \
    ./views/CourseView.cpp \
    ./views/LapView.cpp \
    ./models/PilotListModel.cpp \
    ./models/VideoAssignModel.cpp \
    ./models/VideoSourceModel.cpp \
    ./models/LapTimeRecord.cpp \
    ./models/LapTimeTableModel.cpp \
    ./models/LapTimeCollection.cpp \
    ./models/CourseModel.cpp \
    ./models/LapTimeDetailModel.cpp \
    ./videosources/CameraVideoSource.cpp \
    ./itemdelegates/CheckboxItemDelegate.cpp \
    ./itemdelegates/ColorItemDelegate.cpp \
    ./itemdelegates/AspectItemDelegate.cpp \
    ./itemdelegates/PilotItemDelegate.cpp \
    ./itemdelegates/PointTypeItemDelegate.cpp \
    ./itemdelegates/PointItemDelegate.cpp \
    ./cv/RecognitionThread.cpp \
    ./itemdelegates/ImageItemDelegate.cpp \
    ./itemdelegates/FileItemDelegate.cpp \
    views/AboutView.cpp

HEADERS += \
    ./main/MainWindow.h \
    ./main/Definitions.h \
    ./views/VideoAssignView.h \
    ./views/ViewWindow.h \
    ./views/CameraSettings.h \
    ./views/PreferenceView.h \
    ./views/PilotListView.h \
    ./views/CourseView.h \
    ./views/LapView.h \
    ./models/PilotListModel.h \
    ./models/VideoAssignModel.h \
    ./models/VideoSourceModel.h \
    ./models/LapTimeRecord.h \
    ./models/LapTimeTableModel.h \
    ./models/LapTimeCollection.h \
    ./models/CourseModel.h \
    ./models/LapTimeDetailModel.h \
    ./videosources/IVideoSource.h \
    ./videosources/CameraVideoSource.h \
    ./itemdelegates/CheckboxItemDelegate.h \
    ./itemdelegates/ColorItemDelegate.h \
    ./itemdelegates/ImageItemDelegate.h \
    ./itemdelegates/AspectItemDelegate.h \
    ./itemdelegates/PilotItemDelegate.h \
    ./itemdelegates/PointTypeItemDelegate.h \
    ./itemdelegates/PointItemDelegate.h \
    ./cv/ICVImageSource.h \
    ./cv/RecognitionThread.h \
    ./itemdelegates/FileItemDelegate.h \
    views/AboutView.h

FORMS   += \
    ./views/CameraSettings.ui \
    ./views/PreferenceView.ui \
    ./views/PilotListView.ui \
    ./views/AboutView.ui

RESOURCES += \
    resources/Resources.qrc

RC_ICONS = ./resources/icons/app.ico
ICON     = ./resources/icons/app.icns


win32-msvc
{
    INCLUDEPATH += $$(OPENCV_INSTALL)/include
    LIBS += $$(OPENCV_INSTALL)/x64/vc15/lib/opencv_world401.lib
#    CONFIG(debug,   debug|release) { LIBS += $$(OPENCV_INSTALL)/x64/vc15/lib/opencv_world401d.lib }
#    CONFIG(release, debug|release) { LIBS += $$(OPENCV_INSTALL)/x64/vc15/lib/opencv_world401.lib  }
}

win32-g++
{
    INCLUDEPATH += $$(OPENCV_INSTALL)/include
    LIBS        += $$(OPENCV_INSTALL)/x64/mingw/lib/libopencv_world401.dll.a
}


macx: INCLUDEPATH += /usr/local/include/opencv4
macx: LIBS        += /usr/local/lib/libopencv_world.dylib

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

