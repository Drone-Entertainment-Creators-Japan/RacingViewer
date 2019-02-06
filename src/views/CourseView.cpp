#include "CourseView.h"
#include "ViewWindow.h"
#include "main/Definitions.h"
#include "models/CourseModel.h"
#include "itemdelegates/PointTypeItemDelegate.h"
#include "itemdelegates/PointItemDelegate.h"
#include <QtWidgets/QTableView>

/* ------------------------------------------------------------------------------------------------ */
CourseView::CourseView(QSettings* p_settings, CourseModel* p_course, QWidget* p_parent) : QSplitter(p_parent)
, mp_preview( nullptr )
, mp_table  ( nullptr )
, mp_course ( p_course )
{
    setOrientation(Qt::Horizontal);

    mp_preview = new ViewWindow(p_settings);
    mp_preview->show();
    mp_preview->setMinimumWidth(100);
    addWidget(mp_preview);

    mp_table = new QTableView;
    mp_table->setModel(mp_course);
    mp_table->setItemDelegateForColumn(Definitions::kPointType      , new PointTypeItemDelegate(this));
    mp_table->setItemDelegateForColumn(Definitions::kPointCoordinate, new PointItemDelegate(this));
    addWidget(mp_table);
}

/* ------------------------------------------------------------------------------------------------ */
CourseView::~CourseView(void)
{
}
