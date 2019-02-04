#ifndef COURSEMODEL_H
#define COURSEMODEL_H

#include <QStandardItemModel>
#include <QFile>
#include <QString>

class CourseModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit CourseModel(QObject* p_object=nullptr);
            ~CourseModel(void);

    bool setCourseFilepath(QString filepath);
    QString courseFilepath(void);

    bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole) override;

    bool write(void);

    int pointIndex(int point_id) const;
    int pointType(int index) const;
    int sectionID(int index) const;
    int sectionCount(void) const;

signals:
    void sectionCountChanged(int section_count);

private:
    QString  m_filepath;
    QFile    m_file;
};

#endif /* COURSEMODEL_H */
