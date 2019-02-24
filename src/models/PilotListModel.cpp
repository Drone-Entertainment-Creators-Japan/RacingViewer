#include "PilotListModel.h"

#include "main/Definitions.h"

#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDir>
#include <QModelIndex>
#include <QSettings>
#include <QVariant>
#include <QMetaType>
#include <QSet>
#include <QMap>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
PilotListModel::PilotListModel(QObject* p_parent) : QStandardItemModel(p_parent)
, m_write_enabled( true )
{
    const QMetaObject& defs = Definitions::staticMetaObject;
    m_descripter = defs.enumerator( defs.indexOfEnumerator("PilotDescriptor") );

    setRowCount(1);
    setHeaderData( 0, Qt::Vertical, QString());

    setColumnCount(Definitions::kPilotDescriptorCount);
    setHeaderData(Definitions::kPilotName        , Qt::Horizontal, tr("Name")   );
    setHeaderData(Definitions::kPilotImage       , Qt::Horizontal, tr("Image")  );
    setHeaderData(Definitions::kPilotColor       , Qt::Horizontal, tr("Color")  );
    setHeaderData(Definitions::kCameraAspectRatio, Qt::Horizontal, tr("Aspect") );
}

/* ------------------------------------------------------------------------------------------------ */
PilotListModel::~PilotListModel(void)
{
}

/* ------------------------------------------------------------------------------------------------ */
bool PilotListModel::refresh(void)
{
    /* reset and read all pilot informations */
    QDir directory(m_path);
    QStringList filters;
    filters << "*" + Definitions::kTxtPilotFileExtention;
    directory.setNameFilters( filters );
    QStringList list = directory.entryList(QDir::Files);

    setRowCount(0);
    setRowCount( list.count() + 1 );
    m_write_enabled = false;
    for(int pilot_idx=0; pilot_idx<list.count(); ++pilot_idx)
    {
        QFileInfo fileinfo(directory.filePath(list[pilot_idx]));
        QString pilot_name = fileinfo.fileName();
        pilot_name.remove(pilot_name.count()-Definitions::kTxtPilotFileExtention.count(), Definitions::kTxtPilotFileExtention.count());

        QSettings settings(fileinfo.filePath(), QSettings::IniFormat);
        settings.setValue(m_descripter.valueToKey(Definitions::kPilotName), pilot_name);
        for(int i=0; i<Definitions::kPilotDescriptorCount; ++i)
        {
            setValueOf(pilot_idx, i, settings.value(m_descripter.valueToKey(i)) );
        }
        setHeaderData( pilot_idx, Qt::Vertical, pilot_name);

    }
    setHeaderData( list.count(), Qt::Vertical, "");
    QStandardItemModel::setData(QStandardItemModel::index(list.count(), Definitions::kPilotName)       , "");
    QStandardItemModel::setData(QStandardItemModel::index(list.count(), Definitions::kPilotColor)       , QColor(Qt::white));
    QStandardItemModel::setData(QStandardItemModel::index(list.count(), Definitions::kCameraAspectRatio), Definitions::kTxtAspect_None);

    m_write_enabled = true;
    return true;
}

/* ------------------------------------------------------------------------------------------------ */
bool PilotListModel::setPoolDirectory(QString path)
{
    if( m_path == path ) { return true; }
    m_path = path;

    refresh();

    return true;
}

/* ------------------------------------------------------------------------------------------------ */
QString PilotListModel::poolDirectory(void)
{
    return m_path;
}

/* ------------------------------------------------------------------------------------------------ */
QVariant PilotListModel::valueOf(int pilot_idx, int descriptor_idx) const
{
    QModelIndex index = this->index(pilot_idx, descriptor_idx);
    return this->data(index, Qt::DisplayRole);
}

/* ------------------------------------------------------------------------------------------------ */
bool PilotListModel::setValueOf(int pilot_idx, int descriptor_idx, const QVariant& value)
{
    QModelIndex index = this->index(pilot_idx, descriptor_idx);
    return this->setData(index, value, Qt::EditRole);
}

/* ------------------------------------------------------------------------------------------------ */
bool PilotListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if( role != Qt::EditRole ) { return QStandardItemModel::setData(index, value, role); }

    QVariant current = data(index, Qt::DisplayRole);
    if( value == current ) { return true; }

    int pilot_idx = index.row();
    int desc_idx = index.column();

    /* To keep pilot name to uniqe */
    if( (desc_idx == Definitions::kPilotName) && (! value.toString().isEmpty()) )
    {
        for(int i=0; i<rowCount(); ++i)
        {
            if( value == valueOf(i, Definitions::kPilotName) ) { return false; }
        }
    }

    bool result = QStandardItemModel::setData(index, value, role);
    if( ! result ) { return false; }

    QString pilot_name = valueOf(pilot_idx, Definitions::kPilotName).value<QString>();
    if( desc_idx == Definitions::kPilotName ) { emit pilotDescChanged(current.toString(), desc_idx, value); }
    else                                      { emit pilotDescChanged(pilot_name,         desc_idx, value); }

    QDir directory(poolDirectory());
    if( ! directory.exists() ) { return false; }

    QString filename = headerData(pilot_idx, Qt::Vertical).value<QString>();
    if( pilot_name.isEmpty() && (desc_idx == Definitions::kPilotName) )
    {
        directory.remove(filename + Definitions::kTxtPilotFileExtention);
        if( rowCount() > 1) { removeRow(pilot_idx); }
        return true;
    }

    if( filename.isEmpty() ) { filename = valueOf(pilot_idx, Definitions::kPilotName).value<QString>(); }
    if( filename != pilot_name )
    {
        result = directory.remove(filename + Definitions::kTxtPilotFileExtention);
        filename = pilot_name;
    }

    if( ! m_write_enabled ) { return true; }
    QSettings settings(directory.filePath(filename + Definitions::kTxtPilotFileExtention), QSettings::IniFormat);
    setHeaderData(pilot_idx, Qt::Vertical, filename);
    for(int i=0; i<Definitions::kPilotDescriptorCount; ++i)
    {
        settings.setValue(m_descripter.valueToKey(i), valueOf(pilot_idx, i) );
    }
    if( (pilot_idx+1) == rowCount() )
    {
        insertRow( pilot_idx+1 );
        setHeaderData( pilot_idx+1, Qt::Vertical, QString());
        QStandardItemModel::setData(QStandardItemModel::index(pilot_idx+1, Definitions::kPilotName)        , "");
        QStandardItemModel::setData(QStandardItemModel::index(pilot_idx+1, Definitions::kPilotColor)       , QColor(Qt::white));
        QStandardItemModel::setData(QStandardItemModel::index(pilot_idx+1, Definitions::kCameraAspectRatio), Definitions::kTxtAspect_None);
    }
    settings.sync();
    if( ! directory.exists(filename + Definitions::kTxtPilotFileExtention) )
    {
        emit errorOccurs(filename + QString(tr(" could not be saved. Please ensure the pilots directory is exist and writable.")));
    } else
    {
        emit errorOccurs( QString() );
    }

    return result;
}

/* ------------------------------------------------------------------------------------------------ */
bool PilotListModel::addPilot(const QString& pilot_name)
{
    for(int i=0; i<rowCount(); ++i)
    {
        if( pilot_name != valueOf(i, Definitions::kPilotName).toString() ) { continue; }
        return true;
    }

    return setData(index(rowCount()-1, Definitions::kPilotName), pilot_name);
}

