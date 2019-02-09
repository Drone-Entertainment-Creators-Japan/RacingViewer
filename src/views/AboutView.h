#ifndef ABOUTVIEW_H
#define ABOUTVIEW_H

#include <QWidget>

namespace Ui { class AboutView; }

class AboutView : public QWidget
{
    Q_OBJECT

public:
    explicit AboutView(QWidget *parent = nullptr);
            ~AboutView(void);

private:
    Ui::AboutView* p_ui;
};

#endif /* ABOUTVIEW_H */

