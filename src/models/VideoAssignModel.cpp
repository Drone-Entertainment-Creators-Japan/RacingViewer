#include "VideoAssignModel.h"
#include "models/VideoSourceModel.h"
#include "main/Definitions.h"

#include <QKeySequence>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
VideoAssignModel::VideoAssignModel(VideoSourceModel* p_sources, QObject *parent) :QAbstractTableModel(parent)
, m_layouts( 1 )
, mp_sources( p_sources )
{
    connect(mp_sources, SIGNAL(rowsRemoved (const QModelIndex&, int, int))                              , this, SLOT(submodel_rowsRemoved(const QModelIndex&, int, int)));
    connect(mp_sources, SIGNAL(rowsInserted(const QModelIndex&, int, int))                              , this, SLOT(submodel_rowsInserted(const QModelIndex&, int, int)));
    connect(mp_sources, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int))        , this, SLOT(submodel_rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));
    connect(mp_sources, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)), this, SLOT(submodel_dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

    if( ! mp_sources ) { return; }

    QList<IVideoSource*> list = mp_sources->videoSources();
    for(int i=0; i<mp_sources->rowCount(); ++i)
    {
        VideoSource item;
        item.p_source = list[i];
        item.cutin    = false;
        item.layout_no.resize(m_layouts.count());
        m_assign.insert(i, item);
    }
    for(int i=0; i<m_layouts.count(); ++i)
    {
        m_layouts[i].layout_type = Definitions::kTxtLayout_Auto;
        m_layouts[i].cutin_type  = Definitions::kTxtLayout_Auto;
    }
}

/* ------------------------------------------------------------------------------------------------ */
VideoAssignModel::~VideoAssignModel(void)
{
}

/* ------------------------------------------------------------------------------------------------ */
int VideoAssignModel::rowCount(const QModelIndex& parent) const
{
    return kRowHeader + m_assign.count();
}

/* ------------------------------------------------------------------------------------------------ */
int VideoAssignModel::columnCount(const QModelIndex& parent) const
{
    int source_cols = 0;
    if( mp_sources ) { source_cols = mp_sources->columnCount(); }
    return source_cols + kColHeader + m_layouts.count();
}

/* ------------------------------------------------------------------------------------------------ */
QVariant VideoAssignModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( ! mp_sources ) { return QVariant(); }
    if( role == Qt::TextAlignmentRole ) { return Qt::AlignCenter; }
    if( role != Qt::DisplayRole ) { return QVariant(); }

    if( orientation == Qt::Horizontal )
    {
        switch(section)
        {
            case Definitions::kSourceName:      { return tr("Name");         } break;
            case Definitions::kDeviceID:        { return tr("Devide ID");    } break;
            case Definitions::kPilotAssign:     { return tr("Pilot");        } break;
            case Definitions::kGateRecognition: { return tr("Recogunition"); } break;
            case Definitions::kActiveState:     { return tr("Activate");     } break;
            default: { /* nothing */ } break;
        }

        section -= mp_sources->columnCount();
        if( section < 0 ) { return QVariant(); }
        if( section == kCutIn ) { return tr("Cut In"); }
        return QString().sprintf("%02d", section-kColHeader+1);

    } else if( section < kRowHeader )
    {
        switch(section)
        {
            case kLayoutType: { return tr("Layout");      } break;
            case kCutInType:  { return tr("Cut In Type"); } break;
            default: { /* nothing */ } break;
        }
    } else
    {
        return QString().sprintf("%02d", section-kRowHeader+1);
    }

    return QVariant();
}

/* ------------------------------------------------------------------------------------------------ */
QVariant VideoAssignModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    if( row < 0 ) { return QVariant(); }
    if( col < 0 ) { return QVariant(); }

    int source_cols = 0;
    if( mp_sources ) { source_cols = mp_sources->columnCount(); }
    if(  col < source_cols ) { return mp_sources->data(mp_sources->index(row-kRowHeader, col), role); }

    col -= source_cols;
    if( col < 0 ) { return QVariant(); }
    if( col >= (kColHeader+m_layouts.count()) ) { return QVariant(); }

    if( (row == kLayoutType) && (col >= kColHeader) ) { return m_layouts[col-kColHeader].layout_type; }
    if( (row == kCutInType ) && (col >= kColHeader) ) { return m_layouts[col-kColHeader].cutin_type; }

    row -= kRowHeader;
    if( row <  0                ) { return QVariant(); }
    if( row >= m_assign.count() ) { return QVariant(); }
    if( col == kCutIn )
    {
        if( role != Qt::CheckStateRole ) { return QVariant(); }
        if( m_assign[row].cutin ) { return Qt::Checked; }
        else                      { return Qt::Unchecked; }
    }

    if( role != Qt::DisplayRole ) { return QVariant(); }

    if( m_assign[row].layout_no[col-kColHeader]  <= 0 ) { return QVariant(); }
    return m_assign[row].layout_no[col-kColHeader];
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoAssignModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    int row = index.row();
    int col = index.column();

    if( row < 0 ) { return false; }
    if( col < 0 ) { return false; }

    int source_cols = 0;
    if( mp_sources ) { source_cols = mp_sources->columnCount(); }
    if( col < source_cols ) { return mp_sources->setData(mp_sources->index(row-kRowHeader, col), value, role); }

    col -= source_cols;
    if( col < 0 ) { return false; }
    if( col >= (kColHeader+m_layouts.count()) ) { return false; }

    if( (row == kLayoutType) && (col >= kColHeader) )
    {
        m_layouts[col-kColHeader].layout_type = value.toString();
        emit layoutChanged(col-kColHeader);
    }

    if( (row == kCutInType ) && (col >= kColHeader) )
    {
        m_layouts[col-kColHeader].cutin_type  = value.toString();
        emit layoutChanged(col-kColHeader);
    }

    row -= kRowHeader;
    if( row < 0                ) { return true; }
    if( row >= m_assign.count()) { return false; }
    if( col == kCutIn ) { return setCutin(row, value.toBool()); }

    return setAssign(col-kColHeader, row, value.toInt());
}

/* ------------------------------------------------------------------------------------------------ */
Qt::ItemFlags VideoAssignModel::flags(const QModelIndex &index) const
{
    int row = index.row();
    int col = index.column();

    if( row < 0 ) { return Qt::NoItemFlags; }
    if( col < 0 ) { return Qt::NoItemFlags; }

    int source_cols = 0;
    if( mp_sources ) { source_cols = mp_sources->columnCount(); }
    if( (row < kRowHeader) && (col >= (source_cols+kColHeader)) ) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable; }
    if( col < source_cols ) { return mp_sources->flags(mp_sources->index(row-kRowHeader, col)); }

    row -= kRowHeader;
    col -= source_cols;

    if( row < 0 ) { return Qt::NoItemFlags; }
    if( col < 0 ) { return Qt::NoItemFlags; }
    if( row >= m_assign.count()               ) { return Qt::NoItemFlags; }
    if( col >= (kColHeader+m_layouts.count()) ) { return Qt::NoItemFlags; }
    if( col == kCutIn ) { return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable; }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

/* ------------------------------------------------------------------------------------------------ */
int VideoAssignModel::colToPattern(int col) const
{
    int source_cols = 0;
    if( mp_sources ) { source_cols = mp_sources->columnCount(); }

    return col - kColHeader - source_cols;
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoAssignModel::insertLayout(int pattern)
{
    if( pattern < 0                 ) { return false; }
    if( pattern > m_layouts.count() ) { return false; }

    int col = kColHeader + pattern;
    if( mp_sources ) { col += mp_sources->columnCount(); }

    beginInsertColumns(QModelIndex(), col, col);
    LayoutInfo info;
    info.layout_type = Definitions::kTxtLayout_Auto;
    info.cutin_type  = Definitions::kTxtLayout_Auto;

    m_layouts.insert(pattern, info);

    for(int i=0; i<m_assign.count(); ++i)
    {
        m_assign[i].layout_no.insert(pattern, 0);
    }
    endInsertColumns();
    emit headerDataChanged(Qt::Horizontal, 0, columnCount());

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoAssignModel::removeLayout(int pattern)
{
    if( pattern <  0                 ) { return false; }
    if( pattern >= m_layouts.count() ) { return false; }

    int col = kColHeader + pattern;
    if( mp_sources ) { col += mp_sources->columnCount(); }

    beginRemoveColumns(QModelIndex(), col, col);
    m_layouts.resize( m_layouts.count() - 1);
    for(int i=0; i<m_assign.count(); ++i)
    {
        m_assign[i].layout_no.remove(pattern);
    }
    endRemoveColumns();

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
QString VideoAssignModel::layoutType(int pattern) const
{
    if( pattern <  0                 ) { return ""; }
    if( pattern >= m_layouts.count() ) { return ""; }

    return m_layouts[pattern].layout_type;
}

/* ------------------------------------------------------------------------------------------------ */
QString VideoAssignModel::cutinType(int pattern) const
{
    if( pattern <  0                 ) { return ""; }
    if( pattern >= m_layouts.count() ) { return ""; }

    return m_layouts[pattern].cutin_type;
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoAssignModel::setAssign(int pattern, int video_source, int number)
{
    if( pattern <  0                 ) { return false; }
    if( pattern >= m_layouts.count() ) { return false; }
    if( video_source <  0                ) { return false; }
    if( video_source >= m_assign.count() ) { return false; }

    for(int i=0; i<m_assign.count(); ++i)
    {
        if( m_assign[i].layout_no[pattern] != number ) { continue; }
        m_assign[i].layout_no[pattern] = 0;
        break;
    }

    m_assign[video_source].layout_no[pattern] = number;
    emit assignChanged(pattern);

    QModelIndex index = this->index(kRowHeader+video_source, mp_sources->columnCount() + kColHeader + pattern);
    emit dataChanged(index, index);

    if( ((pattern+1) == m_layouts.count()) && (number > 0) ) { insertLayout(m_layouts.count()); }

    int idx = 0;
    for(; idx<m_assign.count(); ++idx)
    {
        if( m_assign[idx].layout_no[pattern] > 0 ) { break; }
    }
    if( (idx == m_assign.count()) && (m_layouts.count() > 1) )
    {
        removeLayout(pattern);
    }

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
int VideoAssignModel::assign(int pattern, int video_source) const
{
    if( pattern <  0                 ) { return 0; }
    if( pattern >= m_layouts.count() ) { return 0; }
    if( video_source <  0                ) { return 0; }
    if( video_source >= m_assign.count() ) { return 0; }

    return m_assign[video_source].layout_no[pattern];
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoAssignModel::setCutin(int video_source, bool active)
{
    if( video_source <  0                ) { return false; }
    if( video_source >= m_assign.count() ) { return false; }

    for(int i=0; i<m_assign.count(); ++i)
    {
        m_assign[i].cutin = false;
    }

    m_assign[video_source].cutin = active;
    if( active ) { emit cutInChanged(m_assign[video_source].p_source); }
    else         { emit cutInChanged(nullptr); }

    QModelIndex index = this->index(kRowHeader+video_source, mp_sources->columnCount() + kCutIn);
    emit dataChanged(index, index);

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool VideoAssignModel::cutin(int video_source) const
{
    if( video_source <  0                ) { return false; }
    if( video_source >= m_assign.count() ) { return false; }

    return m_assign[video_source].cutin;
}

/* ------------------------------------------------------------------------------------------------ */
int VideoAssignModel::videoSourceCount(void) const
{
    if( ! mp_sources ) { return 0; }
    return mp_sources->videoSources().count();
}

/* ------------------------------------------------------------------------------------------------ */
IVideoSource* VideoAssignModel::videoSource(int video_source)
{
    if( ! mp_sources ) { return nullptr; }
    QList<IVideoSource*> list = mp_sources->videoSources();

    if( video_source <  0            ) { return nullptr; }
    if( video_source >= list.count() ) { return nullptr; }
    return list[video_source];
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignModel::submodel_rowsRemoved(const QModelIndex& parent, int first, int last)
{
    beginRemoveRows(QModelIndex(), first, last);
    for(int video_source=first; video_source<=last; ++video_source)
    {
        setCutin(video_source, false);
        for(int pattern=0; pattern<m_layouts.count(); ++pattern)
        {
            setAssign(pattern, video_source, 0);
        }
    }
    emit activateChanged();
    m_assign.erase(m_assign.begin()+first, m_assign.begin()+last+1);
    endRemoveRows();
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignModel::submodel_rowsInserted(const QModelIndex& parent, int first, int last)
{
    QList<IVideoSource*> sources = mp_sources->videoSources();
    beginInsertRows(QModelIndex(), first, last);
    for(int i=first; i<=last; ++i)
    {
        VideoSource item;
        item.p_source = sources[i];
        item.cutin    = false;
        item.layout_no.resize(m_layouts.count());
        m_assign.insert(i, item);
    }
    endInsertRows();
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignModel::submodel_rowsMoved(const QModelIndex& parent, int start, int end, const QModelIndex& destination, int row)
{
    beginMoveRows(QModelIndex(), start, end, QModelIndex(), row);
    for(int i=0; i<(end-start); ++i)
    {
        m_assign.insert(row+i, m_assign[start+i]);
    }
    m_assign.erase(m_assign.begin()+start, m_assign.begin()+end);
    endMoveRows();
}

/* ------------------------------------------------------------------------------------------------ */
void VideoAssignModel::submodel_dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles)
{
    QModelIndex top_left = index(topLeft.row() + kRowHeader, topLeft.column() );
    QModelIndex bottom_rigtht = index(bottomRight.column() + kRowHeader, bottomRight.column());

    if( topLeft.column() == Definitions::kActiveState )
    {
        emit activateChanged();
    }

    emit dataChanged(top_left, bottom_rigtht, roles);
//    for(int i=0; i<roles.count(); ++i)
//    {
//        setData(top_left, mp_sources->data(topLeft, roles[i]), roles[i]); /* to update check box */
//    }
}
