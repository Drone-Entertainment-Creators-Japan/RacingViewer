#ifndef LAPTIMETABLEMODEL_H
#define LAPTIMETABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QPointer>

class LapTimeRecord;

class LapTimeTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum
    {
        kRow_BestLap,
        kRowHeader
    };

public:
    explicit LapTimeTableModel(QObject* p_parent=nullptr);

    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;

    int rowCount   (const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

    bool setLapTimeList(const QList<LapTimeRecord*>& laptime_list);

public slots:
    void lapTimeChanged(int lap_id, int point_id, const QTime& time);

private:
    QList<QPointer<LapTimeRecord>> m_list;
    int                            m_max_lap_count;
};

#endif /* LAPTIMETABLEMODEL_H */
