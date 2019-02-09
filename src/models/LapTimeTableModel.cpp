#include "LapTimeTableModel.h"
#include "models/LapTimeRecord.h"

#include <QString>
#include <QTime>

/* ------------------------------------------------------------------------------------------------ */
LapTimeTableModel::LapTimeTableModel(QObject* p_parent) : QAbstractTableModel(p_parent)
, m_max_lap_count( 0 )
{
}

/* ------------------------------------------------------------------------------------------------ */
QVariant LapTimeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole ) { return QAbstractTableModel::headerData(section, orientation, role); }

    if( section < 0 ) { return QVariant(); }
    if( orientation == Qt::Vertical)
    {
        if( section == 0 ) { return tr("Best"); }
        return QString().sprintf("% 3d", section);
    }

    if( orientation != Qt::Horizontal ) { return QVariant(); }
    if( section >= m_list.count() ) { return QVariant(); }

    if( ! m_list[section] ) { return "Unknown"; }
    return m_list[section]->name();
}

/* ------------------------------------------------------------------------------------------------ */
int LapTimeTableModel::rowCount(const QModelIndex& parent) const
{
    if( parent.isValid() ) { return 0; }

    return m_max_lap_count + kRowHeader;
}

/* ------------------------------------------------------------------------------------------------ */
int LapTimeTableModel::columnCount(const QModelIndex& parent) const
{
    if( parent.isValid() ) { return 0; }

    return m_list.count();
}

/* ------------------------------------------------------------------------------------------------ */
QVariant LapTimeTableModel::data(const QModelIndex& index, int role) const
{
    if( ! index.isValid() ) { return QVariant(); }
    if( role != Qt::DisplayRole ) { return QVariant(); }

    int pilot_id = index.column();
    int lap_id   = index.row();

    if( pilot_id <  0 ) { return QVariant(); }
    if( pilot_id >= m_list.count() ) { return QVariant(); }

    if( ! m_list[pilot_id] ) { return QVariant(); }
    if( lap_id <   0                               ) { return QVariant(); }
    if( lap_id >= (m_list[pilot_id]->lapCount()+1) ) { return QVariant(); }

    qint32 time = 0;
    if( lap_id == kRow_BestLap ) { time = m_list[pilot_id]->getBestTime(); }
    else                         { time = m_list[pilot_id]->time(lap_id-kRowHeader); }

    if( time <= 0 ) { return QVariant(); }
    return QTime::fromMSecsSinceStartOfDay(time).toString(tr("mm:ss.zzz"));
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeTableModel::setLapTimeList(const QList<LapTimeRecord*>& laptime_list)
{
    beginResetModel();
    for(int i=0; i<m_list.count(); ++i)
    {
        if( ! m_list[i] ) { continue; }
        disconnect(m_list[i], SIGNAL(lapTimeChanged(int, int, qint32)), this, SLOT(lapTimeChanged(int, int, qint32)));
    }
    m_max_lap_count = 0;
    m_list.clear();
    for(int i=0; i<laptime_list.count(); ++i)
    {
        if( ! laptime_list[i] ) { continue; }
        m_list.push_back(laptime_list[i]);
    }

    for(int i=0; i<m_list.count(); ++i)
    {
        if( ! m_list[i] ) { continue; }
        connect(m_list[i], SIGNAL(lapTimeChanged(int, int, qint32)), this, SLOT(lapTimeChanged(int, int, qint32)));

        if( m_list[i]->lapCount() > m_max_lap_count ) { m_max_lap_count = m_list[i]->lapCount(); }
    }
    endResetModel();

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void LapTimeTableModel::lapTimeChanged(int lap_id, int point_id, qint32 msec)
{
    int pilot_id = 0;
    for(; pilot_id<m_list.count(); ++pilot_id)
    {
        if( ! m_list[pilot_id] ) { continue; }
        if( static_cast<QObject*>(m_list[pilot_id]) == sender() ) { break; }
    }
    if( pilot_id >= m_list.count() ) { return; }

    if( m_list[pilot_id]->lapCount() > m_max_lap_count )
    {
        int add_count =  m_list[pilot_id]->lapCount()  - m_max_lap_count;
        beginInsertRows(QModelIndex(), m_max_lap_count, m_max_lap_count+add_count-1);
        endInsertRows();
        m_max_lap_count = m_max_lap_count + add_count;
    }

     QVector<int> roles(2);
     roles[0] = Qt::DisplayRole;
     roles[1] = Qt::EditRole;
     QModelIndex idx = index(lap_id+kRowHeader, pilot_id);
     emit dataChanged(idx, idx, roles);

     idx = index(kRow_BestLap, pilot_id);
     emit dataChanged(idx, idx, roles);
}
