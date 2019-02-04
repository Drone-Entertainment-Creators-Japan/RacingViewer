#ifndef LAPTIMERECORD_H
#define LAPTIMERECORD_H

#include <QObject>

#include <QTime>
#include <QList>
#include <QVector>
#include <QString>

class LapTimeRecord : public QObject
{
    Q_OBJECT
public:
    static const int kSection_LapTime = 0;

public:
    explicit LapTimeRecord(const QString& filepath, QObject* p_parent = nullptr);
            ~LapTimeRecord(void);

    QString  name(void) const;
    bool   rename(QString filepath);

    bool setSectionCount(int count);
    int     sectionCount(int lap_id=-1) const;

    bool clear(void);
    bool open(const QString filepath);
    bool write(void);
    bool close(void);

    QTime setSectionTime(int section, qint64 tick);
    QTime time(int lap_id, int section=kSection_LapTime) const;
    QTime getBestTime(int section=kSection_LapTime, int* p_lap_id=nullptr);

    bool  startLap(qint64 tick, qint64 frequency);
    QTime endLap(qint64 tick);
    bool  cancelLap(void);

    QTime current(qint64 tick) const;
    int  lapCount(void) const;

signals:
    void lapTimeChanged(int lap_id, int section, const QTime& time);

public slots:

private:
    static const QString kFormat;

    QFile                 m_file;
    int                   m_current_lap;
    int                   m_section_count;
    qint64                m_start_tick;
    qint64                m_last_tick;
    qint64                m_frequency;
    QList<QVector<QTime>> m_laps;
};

#endif /* LAPTIMERECORD_H */
