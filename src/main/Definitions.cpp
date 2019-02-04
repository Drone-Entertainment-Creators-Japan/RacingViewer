#include "Definitions.h"

const QString Definitions::kTxtARMarkerType_4X4_50  = tr("( 4 x 4)  50");
const QString Definitions::kTxtARMarkerType_4X4_100 = tr("( 4 x 4) 100");
const QString Definitions::kTxtARMarkerType_4X4_250 = tr("( 4 x 4) 250");


const QString Definitions::kTxtAspect_None = tr("None");
const QString Definitions::kTxtAspect__4_3 = tr(" 4: 3");
const QString Definitions::kTxtAspect_16_9 = tr("16: 9");

const QString Definitions::kTxtLayout_Auto         = tr("Auto");

const QString Definitions::kTxtPoint_None         = tr("-");
const QString Definitions::kTxtPoint_StartAndGoal = tr("Start&Goal");
const QString Definitions::kTxtPoint_Start        = tr("Start");
const QString Definitions::kTxtPoint_Goal         = tr("Goal");
const QString Definitions::kTxtPoint_CheckPoint   = tr("Check point");
const QString Definitions::kTxtPoint_Through      = tr("Through");

const QString Definitions::kTxtPilotFileExtention   = ".pilot";
const QString Definitions::kTxtLapTimeFileExtention = ".laps";
const QString Definitions::kTxtCourseFileExtention  = ".course";
const QString Definitions::kTxtLayoutFileExtention  = ".layout";
const QString Definitions::kTxtCutInFileExtention   = ".cutin";

/* ------------------------------------------------------------------------------------------------ */
QString Definitions::toTextPointType(int type)
{
    QString result;
    switch( type )
    {
        case Definitions::kPoint_StartAndGoal: { result = Definitions::kTxtPoint_StartAndGoal; } break;
        case Definitions::kPoint_Start:        { result = Definitions::kTxtPoint_Start; } break;
        case Definitions::kPoint_Goal:         { result = Definitions::kTxtPoint_Goal; } break;
        case Definitions::kPoint_CheckPoint:   { result = Definitions::kTxtPoint_CheckPoint; } break;
        case Definitions::kPoint_Through:      { result = Definitions::kTxtPoint_Through; } break;
        default:                               { result = Definitions::kTxtPoint_None; } break;
    }

    return result;
}

/* ------------------------------------------------------------------------------------------------ */
int Definitions::toIntPointType(const QString& type)
{
    int result = Definitions::kPoint_None;
    if( type == Definitions::kTxtPoint_StartAndGoal ) { result = Definitions::kPoint_StartAndGoal; }
    if( type == Definitions::kTxtPoint_Start        ) { result = Definitions::kPoint_Start;        }
    if( type == Definitions::kTxtPoint_Goal         ) { result = Definitions::kPoint_Goal;         }
    if( type == Definitions::kTxtPoint_CheckPoint   ) { result = Definitions::kPoint_CheckPoint;   }
    if( type == Definitions::kTxtPoint_Through      ) { result = Definitions::kPoint_Through;      }

    return result;
}

/* ------------------------------------------------------------------------------------------------ */
QString Definitions::fromPointF(const QPointF& point)
{
    return QString().sprintf("(%5.3f, %5.3f)", point.x(), point.y());
}

/* ------------------------------------------------------------------------------------------------ */
QPointF Definitions::toPointF(const QString text)
{
    QByteArray line = text.toUtf8();
    double x = 0;
    double y = 0;
    int cnt = sscanf(line.data(), "%*[^0123456789]%lf%*[^0123456789]%lf", &x, &y);
    if( cnt < 2 ) { return QPointF(); }

    return QPointF(x, y);
}
