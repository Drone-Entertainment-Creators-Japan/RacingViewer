#include "AboutView.h"
#include "ui_AboutView.h"

#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QPixmap>
/* ------------------------------------------------------------------------------------------------ */
AboutView::AboutView(QWidget *parent) : QWidget(parent)
, p_ui(new Ui::AboutView)
{
    p_ui->setupUi(this);

    QFile file(":/rv_license.txt");
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);

    p_ui->p_rv_license->setText( in.readAll() );
    p_ui->p_rv_license->setReadOnly(true);
    file.close();

    file.setFileName( QCoreApplication::applicationDirPath() + "/cv_LICENSE" );
    file.open(QIODevice::ReadOnly);
    in.setDevice(&file);
    p_ui->p_cv_license->setText( in.readAll() );
    p_ui->p_cv_license->setReadOnly(true);
    file.close();

    file.setFileName( QCoreApplication::applicationDirPath() + "/qt_LICENSE" );
    file.open(QIODevice::ReadOnly);
    in.setDevice(&file);
    p_ui->p_qt_license->setText( in.readAll() );
    p_ui->p_qt_license->setReadOnly(true);
    file.close();

    QPixmap cv_logo( QCoreApplication::applicationDirPath() + "/opencv-logo-small.png");
    if( ! cv_logo.isNull() ) { p_ui->p_cv_logo->setPixmap( cv_logo ); }

    QPixmap qt_logo( QCoreApplication::applicationDirPath() + "/logo.png");
    if( ! qt_logo.isNull() ) { p_ui->p_qt_logo->setPixmap( qt_logo ); }
}


/* ------------------------------------------------------------------------------------------------ */
AboutView::~AboutView(void)
{
    delete p_ui;
}
