#ifndef SETTINGDEFINITIONS_H
#define SETTINGDEFINITIONS_H

#include <QObject>
#include <QMetaType>
#include <QString>
#include <QPointF>

class Definitions : public QObject
{
    Q_OBJECT

public:
    enum Options
    {
        kMainWindowSize,
        kEventDir,
        kLayoutDir,
        kPilotDir,
        kPilotListAutoRefresh,
        kCameraFindAndActivate,
        kAutoCreateUnknownPilot,
        kViewerFPS,
        kARMarkerType,
        kSpeechLaptime,
        kOptionsCount
    };
    Q_ENUMS(Options)

    static const QString kTxtARMarkerType_4X4_50;
    static const QString kTxtARMarkerType_4X4_100;
    static const QString kTxtARMarkerType_4X4_250;


    enum PilotDescriptor
    {
        kPilotName,
        kPilotImage,
        kPilotColor,
        kCameraAspectRatio,
        kPilotDescriptorCount
    };
    Q_ENUMS(PilotDescriptor)

    enum VideoSourceDescriptor
    {
        kSourceName,
        kDeviceID,
        kPilotAssign,
        kGateRecognition,
        kActiveState,
        kVideoSourceDescriptorCount
    };
    Q_ENUMS(VideoSourceDescriptor)

    static const int kPilotImageSize = 100;

    enum CameraAspectRatio
    {
        kAspect_None,
        kAspect__4_3,
        kAspect_16_9,
        kAspect_Count
    };

    static const QString kTxtAspect_None;
    static const QString kTxtAspect__4_3;
    static const QString kTxtAspect_16_9;

    enum CoursePointDescriptor
    {
        kPointID,
        kPointType,
        kPointCoordinate,
        kCoursePointDescriptorCount
    };
    Q_ENUMS(CoursePointDescriptor)

    enum CourcePointType
    {
        kPoint_None,
        kPoint_StartAndGoal,
        kPoint_Start,
        kPoint_Goal,
        kPoint_CheckPoint,
        kPoint_Through,
        kPointCount
    };
    Q_ENUM(CourcePointType);

    static const QString kTxtLayout_Auto;

    static const QString kTxtPoint_None;
    static const QString kTxtPoint_StartAndGoal;
    static const QString kTxtPoint_Start;
    static const QString kTxtPoint_Goal;
    static const QString kTxtPoint_CheckPoint;
    static const QString kTxtPoint_Through;

    static const QString kTxtPilotFileExtention;
    static const QString kTxtLapTimeFileExtention;
    static const QString kTxtCourseFileExtention;
    static const QString kTxtLayoutFileExtention;
    static const QString kTxtCutInFileExtention;

    static QString toTextPointType(int type);
    static int     toIntPointType(const QString& type);

    static QString fromPointF(const QPointF& point);
    static QPointF toPointF(const QString text);
};


#endif /* SETTINGDEFINITIONS_H */
