#include "PilotItemDelegate.h"
#include "main/Definitions.h"
#include <QSettings>
#include <QVariant>
#include "models/PilotListModel.h"
#include <QtGui>

/* ------------------------------------------------------------------------------------------------ */
PilotItemDelegate::PilotItemDelegate(QSettings* p_settings, PilotListModel* p_pilot_list, QObject* p_parent) : QItemDelegate(p_parent)
, mp_settings  ( p_settings )
, mp_pilot_list( p_pilot_list )
{
    const QMetaObject& defs = Definitions::staticMetaObject;
    m_options = defs.enumerator( defs.indexOfEnumerator("Options") );
}

/* ------------------------------------------------------------------------------------------------ */
QWidget* PilotItemDelegate::createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QComboBox* p_box = new QComboBox(p_parent);
    if( mp_pilot_list )
    {
        for(int pilot=0; pilot<mp_pilot_list->rowCount(); ++pilot)
        {
            QModelIndex index = mp_pilot_list->index(pilot, Definitions::kPilotName);
            p_box->addItem( mp_pilot_list->data(index).value<QString>() );
        }
    }
    p_box->setEditable(true);

    return p_box;
}

/* ------------------------------------------------------------------------------------------------ */
void PilotItemDelegate::setEditorData(QWidget* p_editor, const QModelIndex& index) const
{
    QString value = index.model()->data(index, Qt::EditRole).value<QString>();

    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( ! p_box ) { return; }
    p_box->setCurrentText(value);
    connect(p_box, SIGNAL(currentIndexChanged(int)), this, SLOT(currentIndexChanged(int)));

}

/* ------------------------------------------------------------------------------------------------ */
void PilotItemDelegate::setModelData(QWidget* p_editor, QAbstractItemModel* p_model, const QModelIndex& index) const
{
    QComboBox* p_box = qobject_cast<QComboBox*>(p_editor);
    if( (! p_box) || (! p_model) ) { return; }

    const QString& text = p_box->currentText();

    bool create_pilot = false;
    if( mp_settings ) { create_pilot = mp_settings->value(m_options.valueToKey(Definitions::kAutoCreateUnknownPilot)).value<bool>(); }
    create_pilot &= (mp_pilot_list != nullptr);
    create_pilot &= (! text.isNull());
    if( create_pilot ) { mp_pilot_list->addPilot( text ); }

    p_model->setData(index, text, Qt::EditRole);
}

/* ------------------------------------------------------------------------------------------------ */
void PilotItemDelegate::updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem&option, const QModelIndex& index ) const
{
    if( p_editor ) { p_editor->setGeometry(option.rect); }
}

/* ------------------------------------------------------------------------------------------------ */
void PilotItemDelegate::currentIndexChanged(int idx)
{
    QComboBox* p_box = static_cast<QComboBox*>(sender());
    if(! p_box) { return; }
    emit commitData(p_box);
}
