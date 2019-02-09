#ifndef LAPTIMECOLLECTION_H
#define LAPTIMECOLLECTION_H


#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QTextToSpeech>
#include <QSound>

class LapTimeRecord;
class CourseModel;

class LapTimeCollection : public QObject
{
    Q_OBJECT

public:
    explicit LapTimeCollection(CourseModel* p_course, QObject* p_parent=nullptr);
            ~LapTimeCollection(void);

    bool setPoolDirectory(QString path);
    QString poolDirectory(void);

    bool setSpeechEnable(bool enable);

    LapTimeRecord*        laptime(const QString& pilot);
    QList<LapTimeRecord*> list(const QStringList& pilots);
    QList<LapTimeRecord*> list(void);

public slots:
    void pilotDescChanged(QString pilot, int descriptor_idx, QVariant value);
    void passedThrough(const QString& pilot, int id, qint64 tick_count, double tick_frequency);
    void sectionCountChanged(int section_count);

private:
    bool                  m_speech_enable;
    QSound                m_sound;
    QTextToSpeech         m_tts;
    QString               m_path;
    QList<LapTimeRecord*> m_list;

    CourseModel*          mp_course;
    int                   m_section_count;
};

#endif /* LAPTIMECOLLECTION_H */
