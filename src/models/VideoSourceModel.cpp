#include "VideoSourceModel.h"
#include "videosources/IVideoSource.h"

#include "main/Definitions.h"
/* ------------------------------------------------------------------------------------------------ */
VideoSourceModel::VideoSourceModel(QObject* p_parent) : QStandardItemModel(0, Definitions::kVideoSourceDescriptorCount, p_parent)
{
    setHeaderData(Definitions::kSourceName , Qt::Horizontal, tr("Name"));
    setHeaderData(Definitions::kDeviceID   , Qt::Horizontal, tr("Device ID"));
    setHeaderData(Definitions::kPilotAssign, Qt::Horizontal, tr("Pilot"));
    setHeaderData(Definitions::kRecognition, Qt::Horizontal, tr("Gate recognition"));
    setHeaderData(Definitions::kActiveState, Qt::Horizontal, tr("Activate"));
}

/* ------------------------------------------------------------------------------------------------ */
VideoSourceModel::~VideoSourceModel(void)
{
    foreach(IVideoSource* p_obj, m_sources)
    {
        delete p_obj;
    }
    m_sources.clear();
}

/* ------------------------------------------------------------------------------------------------ */
Qt::ItemFlags VideoSourceModel::flags(const QModelIndex& index) const
{
    if( index.row() <  0                 ) { return Qt::NoItemFlags; }
    if( index.row() >= m_sources.count() ) { return Qt::NoItemFlags; }
    if( index.column() <  0              ) { return Qt::NoItemFlags; }
    if( index.column() >= Definitions::kVideoSourceDescriptorCount ) { return Qt::NoItemFlags; }
    if( ! m_sources[index.row()] ) { return Qt::NoItemFlags; }

    return m_sources[index.row()]->itemFlags(index.column());
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoSourceModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int row = index.row();
    int col = index.column();

    if( row <  0                 ) { return false; }
    if( row >= m_sources.count() ) { return false; }
    if( col <  0              ) { return false; }
    if( col >= Definitions::kVideoSourceDescriptorCount ) { return false; }
    if( ! m_sources[row] ) { return false; }

    bool result = true;

    Qt::ItemFlags flags = m_sources[row]->itemFlags(col);
    if( flags & Qt::ItemIsUserCheckable )
    {
        if( role != Qt::CheckStateRole ) { return false; }
        result = m_sources[row]->setValue(col, value);
    } else if( role == Qt::EditRole )
    {
        result = m_sources[row]->setValue(col, value);
    }
    if( ! result ) { return false; }

    if( value == QStandardItemModel::data(index, role) ) { return true; }
    return QStandardItemModel::setData(index, value, role);
}

/* ------------------------------------------------------------------------------------------------ */
QVariant VideoSourceModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    int col = index.column();

    if( row <  0                 ) { return QVariant(); }
    if( row >= m_sources.count() ) { return QVariant(); }

    QVariant value = QStandardItemModel::data(index, role);
    if( ! m_sources[row] ) { return value; }
    Qt::ItemFlags flags = m_sources[row]->itemFlags(col);
    if( flags & Qt::ItemIsUserCheckable )
    {
        value = m_sources[row]->value(col);
        if( (role == Qt::CheckStateRole) && (  value.toBool()) ) { return Qt::Checked;   }
        if( (role == Qt::CheckStateRole) && (! value.toBool()) ) { return Qt::Unchecked; }
        return QVariant();
    }

    if( role == Qt::DisplayRole ) { return m_sources[row]->value(col); }
    return value;
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoSourceModel::setVideoSources(const QList<IVideoSource*> sources)
{
    QList<IVideoSource*> new_list = sources;
    for(int row=0; row<m_sources.count(); ++row)
    {
        int idx = new_list.indexOf(m_sources[row]);
        if( idx >= 0 )
        {
            new_list[idx] = nullptr;
        } else
        {
            IVideoSource* p_obj = m_sources[row];
            m_sources.removeAt(row);
            removeRow(row);
            delete p_obj;
            row -= 1;
        }
    }

    int start = m_sources.count();
    new_list.removeAll(nullptr);
    m_sources.append(new_list);
    int  end   = m_sources.count();
    setRowCount(end);
    for(int row=start; row<end; ++row)
    {
        if( ! m_sources[row] ) { continue; }
        for(int col=0; col<Definitions::kVideoSourceDescriptorCount; ++col)
        {
            int role = Qt::EditRole;
            if( col == Definitions::kActiveState ) { role = Qt::CheckStateRole; }
            if( col == Definitions::kRecognition ) { role = Qt::CheckStateRole; }
            setData(index(row,col), m_sources[row]->value(col), role );
        }
    }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
const QList<IVideoSource*> VideoSourceModel::videoSources(void) const
{
    return m_sources;
}

/* ------------------------------------------------------------------------------------------------ */
void VideoSourceModel::pilotDescChanged(QString pilot, int descriptor_idx, QVariant value)
{
    if( descriptor_idx != Definitions::kPilotName ) { return; }

    for(int i=0; i<m_sources.count(); ++i)
    {
        if( ! m_sources[i] ) { continue; }
        QString assign = m_sources[i]->value(Definitions::kPilotAssign).value<QString>();
        if( assign.isEmpty() ) { continue; }
        if( pilot != assign ) { continue; }
        setData(index(i, Definitions::kPilotAssign), value, Qt::EditRole);
        return;
    }

    pilot = value.toString();
    for(int i=0; i<m_sources.count(); ++i)
    {
        if( ! m_sources[i] ) { continue; }
        QString assign = m_sources[i]->value(Definitions::kPilotAssign).value<QString>();
        if( assign.isEmpty() ) { continue; }
        if( pilot != assign ) { continue; }
        setData(index(i, Definitions::kPilotAssign), value, Qt::EditRole);
        return;
    }
}
