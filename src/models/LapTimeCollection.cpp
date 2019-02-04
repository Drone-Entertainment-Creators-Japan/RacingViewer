#include "LapTimeCollection.h"

#include "models/LapTimeRecord.h"
#include "models/CourseModel.h"

#include "main/Definitions.h"
#include <QDir>
#include <QStringList>

/* ------------------------------------------------------------------------------------------------ */
LapTimeCollection::LapTimeCollection(CourseModel* p_course, QObject *parent) : QObject(parent)
, m_speech_enable( false )
, m_sound        ( ":beep.wav" )
, mp_course      ( p_course )
, m_section_count( 0 )
{
    m_tts.setLocale(QLocale(QLocale::English,  QLocale::UnitedStates));

    if( mp_course )
    {
        connect(mp_course, SIGNAL(sectionCountChanged(int )), this, SLOT(sectionCountChanged(int)));
        m_section_count = mp_course->sectionCount();
    }
}

/* ------------------------------------------------------------------------------------------------ */
LapTimeCollection::~LapTimeCollection(void)
{
    for(int i=0; i<m_list.count(); ++i)
    {
        delete m_list[i];
    }
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeCollection::setPoolDirectory(QString path)
{
    m_path = path;

    QDir directory(m_path);
    QStringList filters;
    filters << "*" + Definitions::kTxtPilotFileExtention;
    directory.setNameFilters( filters );
    QStringList list = directory.entryList(QDir::Files);

    for(int i=0; i<m_list.count(); ++i) { delete m_list[i]; }
    m_list.clear();

    for(int pilot_idx=0; pilot_idx<list.count(); ++pilot_idx)
    {
        LapTimeRecord* p_record = new LapTimeRecord(directory.filePath(list[pilot_idx]), this);
        p_record->setSectionCount(m_section_count);
        m_list.push_back( p_record );
    }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
QString LapTimeCollection::poolDirectory(void)
{
    return m_path;
}

bool LapTimeCollection::setSpeechEnable(bool enable)
{
    m_speech_enable = enable;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
LapTimeRecord* LapTimeCollection::laptime(const QString& pilot)
{
    if( pilot.isEmpty() ) { return nullptr; }
    for(int i=0; i<m_list.count(); ++i)
    {
        if( ! m_list[i] ) { continue; }
        if( m_list[i]->name() != pilot ) { continue; }
        return m_list[i];
    }

    QString filepath = m_path + "/" + pilot + Definitions::kTxtLapTimeFileExtention;
    LapTimeRecord* p_record = new LapTimeRecord(filepath, this);
    p_record->setSectionCount( m_section_count );
    m_list.push_back(p_record);

    return p_record;
}

/* ------------------------------------------------------------------------------------------------ */
QList<LapTimeRecord*> LapTimeCollection::list(const QStringList& pilots)
{
    QList<LapTimeRecord*> result;
    foreach(const QString& pilot_name, pilots)
    {
        if( pilot_name.isEmpty() ) { continue; }
        int i = 0;
        for(; i<m_list.count(); ++i)
        {
            if( ! m_list[i] ) { continue; }
            if( m_list[i]->name() != pilot_name ) { continue; }
            result.push_back(m_list[i]);
            break;
        }
        if( i < m_list.count() ) { continue; }

        QString filepath = m_path + "/" + pilot_name + Definitions::kTxtLapTimeFileExtention;
        LapTimeRecord* p_record = new LapTimeRecord(filepath, this);
        p_record->setSectionCount( m_section_count );
        m_list.push_back(p_record);
        result.push_back(p_record);
    }
    return result;
}

/* ------------------------------------------------------------------------------------------------ */
QList<LapTimeRecord*> LapTimeCollection::list(void)
{
    return m_list;
}
    /* ------------------------------------------------------------------------------------------------ */
void LapTimeCollection::pilotDescChanged(QString pilot, int descriptor_idx, QVariant value)
{
    if( descriptor_idx != Definitions::kPilotName ) { return; }

    int id = 0;
    for(; id<m_list.count(); ++id)
    {
        if( ! m_list[id] ) { continue; }
        if( m_list[id]->name() != pilot ) { continue; }
        break;
    }
    if( id >= m_list.count() ) { return; }
    if( ! m_list[id] ) { return; }

    QString new_name = value.toString();

    if( new_name.isEmpty() )
    {
        delete m_list[id];
        m_list.removeAt(id);
    } else
    {
        m_list[id]->rename(m_path + "/" + new_name + Definitions::kTxtLapTimeFileExtention );
    }

}

/* ------------------------------------------------------------------------------------------------ */
void LapTimeCollection::passedThrough(const QString& pilot, int id, qint64 tick_count, qint64 tick_frequency)
{
    if( ! mp_course ) { return; }
    LapTimeRecord* p_record = nullptr;

    for(int i=0; i<m_list.count(); ++i)
    {
        if( ! m_list[i] ) { continue; }
        if( m_list[i]->name() != pilot ) { continue; }
        p_record = m_list[i];
        break;
    }
    if( ! p_record )
    {
        QString filepath = m_path + "/" + pilot + Definitions::kTxtLapTimeFileExtention;
        p_record = new LapTimeRecord(filepath, this);
        p_record->setSectionCount( mp_course->sectionCount() );
        m_list.push_back(p_record);
    }

    int point_idx  = mp_course->pointIndex(id);
    int point_type = mp_course->pointType(point_idx);
    int section_id = mp_course->sectionID(point_idx);
    QTime time;
    switch( point_type )
    {
        case Definitions::kPoint_StartAndGoal:
        {
            time = p_record->endLap(tick_count);
            p_record->startLap(tick_count, tick_frequency);
        } break;
        case Definitions::kPoint_Start:        { p_record->startLap(tick_count, tick_frequency); } break;
        case Definitions::kPoint_CheckPoint:   { p_record->setSectionTime(section_id, tick_count); } break;
        case Definitions::kPoint_Goal:         { time = p_record->endLap(tick_count); } break;
        default: { /* nothing */ } break;
    }

    if( time.isNull() ) { return; }
    m_sound.play();

    if( ! m_speech_enable ) { return; }
    m_tts.say(pilot + ". " + time.toString("s.zzz"));
}

/* ------------------------------------------------------------------------------------------------ */
void LapTimeCollection::sectionCountChanged(int section_count)
{
    if( section_count == m_section_count) { return; }
    m_section_count = section_count;

    for(int i=0; i<m_list.count(); ++i)
    {
        if( ! m_list[i] ) { continue; }
        m_list[i]->setSectionCount(section_count);
    }
}

