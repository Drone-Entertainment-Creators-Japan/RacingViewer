#ifndef PILOTLISTMODEL_H
#define PILOTLISTMODEL_H

#include <QStandardItemModel>
#include <QMetaEnum>

class PilotListModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit PilotListModel(QObject* p_parent=nullptr);
            ~PilotListModel(void) override;

    bool refresh(void);

    bool setPoolDirectory(QString path);
    QString poolDirectory(void);

    QVariant    valueOf(int pilot_idx, int descriptor_idx) const;
    bool     setValueOf(int pilot_idx, int descriptor_idx, const QVariant& value);

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    bool addPilot(const QString& pilot_name);
signals:
    void errorOccurs(const QString& message);

    void pilotDescChanged(QString pilot, int descriptor_idx, QVariant value);

private:
    bool      m_write_enabled;
    QMetaEnum m_descripter;
    QString   m_path;
};

#endif /* PILOTLISTMODEL_H */
