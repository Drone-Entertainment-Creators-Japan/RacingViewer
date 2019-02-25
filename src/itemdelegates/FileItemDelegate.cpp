#include "FileItemDelegate.h"
#include "main/Definitions.h"
#include <QComboBox>
#include <QPainter>
#include <QSettings>
#include <QDir>
#include <QMetaEnum>
#include <QDebug>

/* ------------------------------------------------------------------------------------------------ */
FileItemDelegate::FileItemDelegate(QSettings* p_settings, const QString& extention, QObject* p_parent) : QAbstractItemDelegate(p_parent)
, mp_settings( p_settings )
, m_extention( extention  )
{
}

/* ------------------------------------------------------------------------------------------------ */
QWidget* FileItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const QMetaObject& defs = Definitions::staticMetaObject;
    QMetaEnum options = defs.enumerator( defs.indexOfEnumerator("Options") );
    QString path = mp_settings->value(options.valueToKey(Definitions::kLayoutDir)).value<QString>();

    QDir directory(path);
    QStringList filters;
    filters << "*" + m_extention;
    directory.setNameFilters( filters );
    QStringList list = directory.entryList(QDir::Files);

    QComboBox* p_box = new QComboBox(p_parent);
    p_box->addItem( Definitions::kTxtLayout_Auto );
    foreach(const QString& item, list)
    {
        int left = item.count()-m_extention.count();
        if( left <= 0 ) { continue; }
        p_box->addItem( item.left( left ) );
    }

    return p_box;
}

/* ------------------------------------------------------------------------------------------------ */
void FileItemDelegate::setEditorData(QWidget* p_editor, const QModelIndex& index) const
{
    QString text = index.model()->data(index, Qt::EditRole).toString();

    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( ! p_box ) { return; }
    p_box->setCurrentText(text);
    connect(p_box, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));
}

/* ------------------------------------------------------------------------------------------------ */
void FileItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( (! p_box) || (! p_model) ) { return; }

    p_model->setData(index, p_box->currentText(), Qt::EditRole);
}

/* ------------------------------------------------------------------------------------------------ */
void FileItemDelegate::updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem&option, const QModelIndex& index ) const
{
    if( p_editor ) { p_editor->setGeometry(option.rect); }
}

/* ------------------------------------------------------------------------------------------------ */
void FileItemDelegate::paint(QPainter* p_painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if( ! p_painter ) { return; }
    QString text = index.data().toString();
    QRect rect = option.rect;
    if( ! ( Qt::ItemIsEditable & index.flags() ) ) { p_painter->fillRect(option.rect, Qt::gray); }
    p_painter->drawText(rect, Qt::AlignCenter, text, &rect);
}

/* ------------------------------------------------------------------------------------------------ */
QSize FileItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QFont font;
    QFontMetrics metrics(font);

    return metrics.boundingRect(index.data().toString()).size();
}

/* ------------------------------------------------------------------------------------------------ */
void FileItemDelegate::currentIndexChanged(int idx)
{
    QComboBox* p_box = static_cast<QComboBox*>(sender());
    if(! p_box) { return; }
    emit commitData(p_box);
}
