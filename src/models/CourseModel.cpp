#include "CourseModel.h"
#include "main/Definitions.h"
#include <QVariant>
#include <QPointF>
#include <QByteArray>
#include <QTextStream>

#include <stdio.h>

/* ------------------------------------------------------------------------------------------------ */
CourseModel::CourseModel(QObject* p_object) : QStandardItemModel(p_object)
{
    setColumnCount(Definitions::kCoursePointDescriptorCount);
    setHeaderData(Definitions::kPointID          , Qt::Horizontal, tr("ID")       );
    setHeaderData(Definitions::kPointType        , Qt::Horizontal, tr("Type")     );
    setHeaderData(Definitions::kPointCoordinate  , Qt::Horizontal, tr("Position") );

    setRowCount(1);
    setHeaderData( 0, Qt::Vertical, QString("  1"));
    setData(index(0, Definitions::kPointID        ), 0);
    setData(index(0, Definitions::kPointType      ), Definitions::kPoint_StartAndGoal);
    setData(index(0, Definitions::kPointCoordinate), QPointF(0.5, 0.5) );
}

/* ------------------------------------------------------------------------------------------------ */
CourseModel::~CourseModel(void)
{
    write();
    m_file.close();
}

/* ------------------------------------------------------------------------------------------------ */
bool CourseModel::setCourseFilepath(QString filepath)
{
    m_file.close();

    m_filepath = filepath;
    m_file.setFileName(m_filepath);
    bool is_exist = m_file.exists();
    bool is_ok = m_file.open(QIODevice::ReadWrite | QIODevice::Text);
    if( ! is_ok ) { return false; }

    if( ! is_exist ) { return true; }
    removeRows(0, rowCount());

    int max_id = 0;
    while( ! m_file.atEnd() )
    {
        QByteArray line = m_file.readLine();
        if( line.isEmpty() ) { continue; }

        char* p_line = line.data();
        char* p_line_end = p_line + line.count();
        char* p_begin = p_line;
        int desc_id = 0;
        int id =-1;
        int type = Definitions::kPoint_None;
        QPointF point;
        for(; p_line<p_line_end; ++p_line)
        {
            if( p_line[0] != ':' ) { continue; }
            p_line[0] = 0;
            switch( desc_id )
            {
                case Definitions::kPointID:         { id    = atoi(p_begin); } break;
                case Definitions::kPointType:       { type  = Definitions::toIntPointType( QString::fromUtf8(p_begin) ); } break;
                case Definitions::kPointCoordinate: { point = Definitions::toPointF( QString(p_begin) ); } break;
                default: { /* nothing */ } break;
            }
            p_begin = p_line + 1;
            desc_id += 1;
            if( max_id < id ) { max_id = id; }
        }
        if( desc_id < Definitions::kCoursePointDescriptorCount ) { continue; }
        int new_row = rowCount();
        insertRow(new_row);
        setHeaderData(new_row, Qt::Vertical, QString().sprintf("% 3d", new_row + 1));
        QStandardItemModel::setData(index(new_row, Definitions::kPointID        ), id    );
        QStandardItemModel::setData(index(new_row, Definitions::kPointType      ), type  );
        QStandardItemModel::setData(index(new_row, Definitions::kPointCoordinate), point );
    }

    int new_row = rowCount();
    insertRow(new_row);
    setHeaderData(new_row, Qt::Vertical, QString().sprintf("% 3d", new_row + 1));
    QStandardItemModel::setData(index(new_row, Definitions::kPointID        ), max_id+1);
    QStandardItemModel::setData(index(new_row, Definitions::kPointType      ), Definitions::kTxtPoint_None);
    QStandardItemModel::setData(index(new_row, Definitions::kPointCoordinate), QPointF(0.5, 0.5) );

    emit sectionCountChanged(sectionCount());

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
QString CourseModel::courseFilepath(void)
{
    return m_filepath;
}

/* ------------------------------------------------------------------------------------------------ */
bool CourseModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    int row     = idx.row();
    int desc_id = idx.column();

    if( desc_id == Definitions::kPointID )
    {
        for(int i=0; i<rowCount(); ++i)
        {
            QModelIndex it = index(i, Definitions::kPointID);
            if( value.toInt() != data( it ).toInt() ) { continue; }
            QStandardItemModel::setData(it, -1);
        }
    }

    if( desc_id == Definitions::kPointCoordinate )
    {
        /* limiting for file format */
        QPointF point = value.toPointF();
        if( point.x() < 0. ) { point.setX(0.); }
        if( point.x() > 1. ) { point.setX(1.); }
        if( point.y() < 0. ) { point.setY(0.); }
        if( point.y() > 1. ) { point.setY(1.); }
    }

    bool result = QStandardItemModel::setData(idx, value, role);

    if( desc_id == Definitions::kPointType )
    {
        if( row == (rowCount()-1) )
        {
            int new_row = rowCount();
            int max_id = 0;
            for(int i=0; i<rowCount(); ++i)
            {
                QModelIndex it = index(i, Definitions::kPointID);
                int id = data( it ).toInt();
                if( id > max_id ) { max_id = id; }
            }
            insertRow(new_row);
            setHeaderData(new_row, Qt::Vertical, QString().sprintf("% 3d", new_row + 1));
            QStandardItemModel::setData(index(new_row, Definitions::kPointID        ), max_id+1);
            QStandardItemModel::setData(index(new_row, Definitions::kPointType      ), Definitions::kPoint_None);
            QStandardItemModel::setData(index(new_row, Definitions::kPointCoordinate), QPointF(0.5, 0.5) );
        }
        emit sectionCountChanged(sectionCount());
    }

    return result;
}

/* ------------------------------------------------------------------------------------------------ */
bool CourseModel::write(void)
{
    if( ! m_file.isOpen() ) { return false; }
    m_file.seek(0);
    QTextStream out(&m_file);

    out.setCodec("UTF-8");
    out.reset();
    for(int row=0; row<rowCount(); ++row)
    {
        int type = data(index(row, Definitions::kPointType)).toInt();
        if( type <= Definitions::kPoint_None ) { continue; }
        if( type >= Definitions::kPointCount ) { continue; }

        out << data(index(row, Definitions::kPointID)).toString() << ":";
        out << Definitions::toTextPointType(type) << ":";
        out << Definitions::fromPointF(data(index(row, Definitions::kPointCoordinate)).value<QPointF>()) << ":";
        out << endl;
    }
    out.flush();

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
int CourseModel::pointIndex(int point_id) const
{
    for(int row=0; row<rowCount(); ++row)
    {
        if( data(index(row, Definitions::kPointID)).toInt() != point_id) { continue; }
        return row;
    }
    return -1;
}

/* ------------------------------------------------------------------------------------------------ */
int CourseModel::pointType(int idx) const
{
    if( idx < 0 ) { return Definitions::kPoint_None; }
    if( idx >= rowCount() ) { return Definitions::kPoint_None; }

    return data(index(idx, Definitions::kPointType)).toInt();
}

/* ------------------------------------------------------------------------------------------------ */
int CourseModel::sectionID(int idx) const
{
    if( idx < 0 ) { return Definitions::kPoint_None; }
    if( idx >= rowCount() ) { return Definitions::kPoint_None; }

    int section_id = 1;
    for(int i=0; i<idx; ++i)
    {
        if( data(index(i, Definitions::kPointType)).toInt() != Definitions::kPoint_CheckPoint) { continue; }
        section_id += 1;
    }

    return section_id;
}

/* ------------------------------------------------------------------------------------------------ */
int CourseModel::sectionCount(void) const
{
    int section_id = 1;
    for(int i=0; i<rowCount(); ++i)
    {
        if( data(index(i, Definitions::kPointType)).toInt() != Definitions::kPoint_CheckPoint) { continue; }
        section_id += 1;
    }

    return section_id;
}
