#ifndef COURSEVIEW_H
#define COURSEVIEW_H

#include <QtWidgets/QSplitter>

class QSettings;
class ViewWindow;
class QTableView;
class CourseModel;

class CourseView : public QSplitter
{
    Q_OBJECT

public:
    explicit CourseView(QSettings* p_settings, CourseModel* p_course, QWidget* P_parent=nullptr);
            ~CourseView(void);

private:
    ViewWindow*  mp_preview;
    QTableView*  mp_table;

    CourseModel* mp_course;
};

#endif /* COURSEVIEW_H */
