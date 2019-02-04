#ifndef LAPTIMEDETAILMODEL_H
#define LAPTIMEDETAILMODEL_H

#include <QAbstractTableModel>
#include <QPointer>

class LapTimeRecord;

class LapTimeDetailModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    struct Item
    {
        QPointer<LapTimeRecord> p_record;
        int                      lap_id;
    };

public:
    explicit LapTimeDetailModel(QObject* P_parent=nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

    bool setItemList(const QList<Item>& items);

public slots:
    void lapTimeChanged(int lap_id, int section, const QTime& time);

private:
    QList<Item> m_items;
};

#endif /* LAPTIMEDETAILMODEL_H */
