#include "LapTimeDetailModel.h"
#include "models/LapTimeRecord.h"

/* ------------------------------------------------------------------------------------------------ */
LapTimeDetailModel::LapTimeDetailModel(QObject* p_parent) : QAbstractTableModel(p_parent)
{

}

/* ------------------------------------------------------------------------------------------------ */
QVariant LapTimeDetailModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( role != Qt::DisplayRole ) { return QAbstractTableModel::headerData(section, orientation, role); }

    if( section < 0 ) { return QVariant(); }
    if( orientation == Qt::Vertical)
    {
        if( section == 0 ) { return tr("Lap time"); }
        return QString().sprintf("Section % 2d", section);
    }

    if( orientation != Qt::Horizontal ) { return QVariant(); }
    if( section >= m_items.count() ) { return QVariant(); }

    LapTimeRecord* p_record = m_items[section].p_record;
    int            lap_id   = m_items[section].lap_id;
    if( ! p_record ) { return tr("Unknown"); }
    if( lap_id < 0 ) { return tr("Unknown"); }

    return p_record->name() + QString().sprintf(" lap %d", lap_id+1);
}

/* ------------------------------------------------------------------------------------------------ */
int LapTimeDetailModel::rowCount(const QModelIndex& parent) const
{
    if( parent.isValid() ) { return 0; }

    int max_section_count = 0;
    for(int i=0; i<m_items.count(); ++i)
    {
        LapTimeRecord* p_record = m_items[i].p_record;
        int            lap_id   = m_items[i].lap_id;
        if( ! p_record ) { continue; }
        if( lap_id < 0 ) { continue; }

        int section_count = p_record->sectionCount( m_items[i].lap_id );
        if( section_count > max_section_count ) { max_section_count = section_count; }
    }

    return max_section_count;
}

/* ------------------------------------------------------------------------------------------------ */
int LapTimeDetailModel::columnCount(const QModelIndex& parent) const
{
    if( parent.isValid() ) { return 0; }

    return m_items.count();
}

/* ------------------------------------------------------------------------------------------------ */
QVariant LapTimeDetailModel::data(const QModelIndex& index, int role) const
{
    if( ! index.isValid() ) { return QVariant(); }
    if( role != Qt::DisplayRole ) { return QVariant(); }

    int item_id = index.column();
    int section = index.row();

    if( item_id <  0 ) { return QVariant(); }
    if( item_id >= m_items.count() ) { return QVariant(); }

    LapTimeRecord* p_record = m_items[item_id].p_record;
    int            lap_id   = m_items[item_id].lap_id;
    if( ! p_record   ) { return QVariant(); }
    if(   lap_id < 0 ) { return QVariant(); }
    if( section <   0                             ) { return QVariant(); }
    if( section >= p_record->sectionCount(lap_id) ) { return QVariant(); }

    QTime time = p_record->time(lap_id, section);
    return time.toString(tr("mm:ss.zzz"));
}

/* ------------------------------------------------------------------------------------------------ */
bool LapTimeDetailModel::setItemList(const QList<Item>& items)
{
    beginResetModel();
    for(int i=0; i<m_items.count(); ++i)
    {
        LapTimeRecord* p_record = m_items[i].p_record;
        if( ! p_record ) { continue; }
        disconnect(p_record, SIGNAL(lapTimeChanged(int, int, const QTime&)), this, SLOT(lapTimeChanged(int, int, const QTime&)));
    }
    m_items = items;

    for(int i=0; i<m_items.count(); ++i)
    {
        LapTimeRecord* p_record = m_items[i].p_record;
        if( ! p_record ) { continue; }
        disconnect(p_record, SIGNAL(lapTimeChanged(int, int, const QTime&)), this, SLOT(lapTimeChanged(int, int, const QTime&)));
    }
    endResetModel();

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
void LapTimeDetailModel::lapTimeChanged(int lap_id, int section, const QTime& time)
{
    for(int i=0; i<m_items.count(); ++i)
    {
        LapTimeRecord* p_record   = m_items[i].p_record;
        if( static_cast<QObject*>( p_record ) != sender() ) { continue; }
        if( m_items[i].lap_id != lap_id ) { continue; }

        QModelIndex idx = index(section, i);
        QVector<int> roles(2);
        roles[0] = Qt::EditRole;
        roles[1] = Qt::DisplayRole;
        emit dataChanged( idx, idx, roles );

        return;
    }
}

