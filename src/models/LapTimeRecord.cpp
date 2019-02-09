#include "LapTimeRecord.h"
#include "main/Definitions.h"
#include <QTextStream>
#include <QFileInfo>
#include <QDir>

const QString LapTimeRecord::kFormat = "mm:ss.zzz";

/* ------------------------------------------------------------------------------------------------ */
LapTimeRecord::LapTimeRecord(const QString& filepath, QObject* p_parent) : QObject(p_parent)
, m_current_lap  ( - 1 )
, m_section_count(   1 )
{
    open(filepath);
}

/* ------------------------------------------------------------------------------------------------ */
LapTimeRecord::~LapTimeRecord(void)
{
    cancelLap();
    write();
    close();
}

/* ------------------------------------------------------------------------------------------------ */
QString LapTimeRecord::name(void) const
{
    QFileInfo info(m_file.fileName());
    QString name = info.fileName();
    int left = name.count() - Definitions::kTxtLapTimeFileExtention.count();
    if( left <= 0 ) { return ""; }
    return name.left(left);
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::rename(QString filepath)
{
    if( filepath.isNull() )
    {
        m_file.remove();
        return true;
    }
    return m_file.rename(filepath);
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::setSectionCount(int count)
{
    m_section_count = count;

    if( m_current_lap <  0              ) { return true; }
    if( m_current_lap >= m_laps.count() ) { return true; }

    m_laps[m_current_lap].resize(m_section_count);
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
int LapTimeRecord::sectionCount(int lap_id) const
{
    if( lap_id < 0 ) { return m_section_count; }
    if( lap_id >= m_laps.count() ) { return 0; }

    return m_laps[lap_id].count();
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::clear(void)
{
    m_laps.clear();
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::open(const QString filepath)
{
    QList<QVector<qint32>> laps = m_laps;
    m_file.setFileName(filepath);
    m_file.open(QIODevice::ReadWrite | QIODevice::Text);

    while( ! m_file.atEnd() )
    {
        QByteArray line = m_file.readLine();
        if( line.isEmpty() ) { continue; }

        QVector<qint32> times;
        char* p_line = line.data();
        char* p_line_end = p_line + line.count();
        char* p_begin = p_line;
        for(; p_line<p_line_end; ++p_line)
        {
            if( p_line[0] != ',' ) { continue; }
            p_line[0] = 0;
            QTime time(0, 0, 0, 0);
            times.push_back( static_cast<qint32>(time.msecsTo( QTime::fromString( QString(p_begin), kFormat) )) );
            p_begin = p_line + 1;
        }
        m_laps.append( times );
    }

    m_current_lap += laps.count();
    m_laps.append( laps );

    for(int lap_id=0; lap_id<m_laps.count(); ++lap_id)
    {
        int point_count = m_laps[lap_id].count();
        for(int point_id=0; point_id<m_section_count; ++point_id)
        {
            qint32 time = 0;
            if( point_id < point_count ) { time = m_laps[lap_id][point_id]; }
            emit lapTimeChanged(lap_id, point_id, time);
        }
    }

   return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::write(void)
{
    m_file.seek(0);

    QTextStream out(&m_file);
    out.setCodec("UTF-8");

    foreach(const QVector<qint32>& times, m_laps)
    {
        if( times.empty() ) { continue; }

        for(int i=0; i<times.count(); ++i)
        {
            out << QTime::fromMSecsSinceStartOfDay(times[i]).toString(kFormat) <<  ",";
        }
        out << endl;
    }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::close(void)
{
    m_file.close();
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
qint32 LapTimeRecord::setSectionTime(int section, qint64 tick)
{
    if( m_current_lap < 0 ) { return 0; }
    if( m_current_lap >= m_laps.count() ) { return 0;}
    if( section < 1 ) { return 0; }
    if( section >= m_laps[m_current_lap].count() ) { return 0; }
    if( m_frequency <= 0 ) { return 0; }

    if( m_laps[m_current_lap][section] > 0 ) { return m_laps[m_current_lap][section]; }

    m_laps[m_current_lap][section] = static_cast<qint32>( (tick-m_last_tick)*1000./m_frequency + 0.5);
    m_last_tick = tick;

    emit lapTimeChanged(m_current_lap, section, m_laps[m_current_lap][section]);

    return m_laps[m_current_lap][section];
}

/* ------------------------------------------------------------------------------------------------ */
qint32 LapTimeRecord::time(int lap_id, int section) const
{
    if( lap_id < 0 ) { return 0; }
    if( lap_id > m_laps.count( ) ) { return 0; }
    if( section < 0 ) { return 0; }
    if( section >= m_laps[lap_id].count() ) { return 0; }

    return m_laps[lap_id][section];
}

/* ------------------------------------------------------------------------------------------------ */
qint32 LapTimeRecord::getBestTime(int section, int* p_lap_id)
{
    if( section < 0 ) { return 0; }
    int lap_id = -1;
    qint32 result = INT32_MAX;

    for(int i=0; i<m_laps.count(); ++i)
    {
        if( section >= m_laps[i].count() ) { continue; }
        if( m_laps[i][section] <= 0      ) { continue; }
        if( result < m_laps[i][section]  ) { continue; }

        result = m_laps[i][section];
        lap_id = i;
    }
    if( lap_id < 0) { result = 0; }
    if( p_lap_id ) { *p_lap_id = lap_id; }
    return result;
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::startLap(qint64 tick, double frequency)
{
    if( m_section_count <= 0 ) { return false; }

    m_laps.append( QVector<qint32>(m_section_count));
    m_current_lap = m_laps.count() - 1;
    m_start_tick = tick;
    m_last_tick  = tick;
    m_frequency  = frequency;
    emit lapTimeChanged(m_current_lap, kSection_LapTime, m_laps[m_current_lap][kSection_LapTime]);

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
qint32 LapTimeRecord::endLap(qint64 tick)
{
    if( m_current_lap < 0 ) { return 0; }
    if( m_current_lap >= m_laps.count() ) { return 0;}
    if( m_frequency <= 0 ) { return 0; }
    int current = m_current_lap;
    m_current_lap = -1;

    m_laps[current][kSection_LapTime] = static_cast<qint32>( (tick-m_start_tick)*1000./m_frequency + 0.5);

    QTextStream out(&m_file);
    out.setCodec("UTF-8");

    QVector<qint32>& lap = m_laps[current];
    out << QTime::fromMSecsSinceStartOfDay(lap[0]).toString(kFormat);
    for(int i=1; i<lap.count(); ++i)
    {
        out <<  "," << QTime::fromMSecsSinceStartOfDay(lap[i]).toString(kFormat);
    }
    out << endl;

    emit lapTimeChanged(current, kSection_LapTime, m_laps[current][kSection_LapTime]);

    return m_laps[current][kSection_LapTime];
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeRecord::cancelLap(void)
{
    if( m_current_lap < 0 ) { return true; }
    if( m_current_lap >= m_laps.count() ) { return true;}
    if( m_frequency <= 0 ) { return true; }

    m_laps.removeAt(m_current_lap);

    for(int i=0; i<m_section_count; ++i)
    {
        emit lapTimeChanged(m_current_lap, i, 0);
    }
    m_current_lap = -1;

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
qint32 LapTimeRecord::current(qint64 tick) const
{
    if( m_current_lap < 0 ) { return 0; }
    if( m_current_lap >= m_laps.count() ) { return 0;}
    if( m_frequency <= 0 ) { return 0; }

    return static_cast<qint32>( (tick-m_start_tick)*1000./m_frequency + 0.5);
}

/* ------------------------------------------------------------------------------------------------ */
int LapTimeRecord::lapCount(void) const
{
    return m_laps.count();
}
