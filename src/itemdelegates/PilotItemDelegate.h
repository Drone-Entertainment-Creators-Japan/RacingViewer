#ifndef PILOTITEMDELEGATE_H
#define PILOTITEMDELEGATE_H


#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QComboBox>
#include <QMetaEnum>

class QSettings;
class PilotListModel;

class PilotItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    PilotItemDelegate(QSettings* p_settings, PilotListModel* p_pilot_list, QObject* p_parent=nullptr);

    QWidget* createEditor(QWidget* p_parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

    void setEditorData(QWidget* p_editor, const QModelIndex& index) const;
    void setModelData(QWidget* p_editor, QAbstractItemModel* p_model,const QModelIndex& index) const;

    void updateEditorGeometry(QWidget* p_editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

private:
    QMetaEnum       m_options;
    QSettings*      mp_settings;
    PilotListModel* mp_pilot_list;
};

#endif /* ASPECTITEPILOTITEMDELEGATE_HMDELEGATE_H */
