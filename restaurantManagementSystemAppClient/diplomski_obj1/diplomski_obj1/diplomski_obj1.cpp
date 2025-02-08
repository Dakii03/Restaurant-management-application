#include "diplomski_obj1.h"
#include "table_management.h"

diplomski_obj1::diplomski_obj1(QWidget* parent)
    : QMainWindow(parent), tableManagementWidget(nullptr)
{
    ui.setupUi(this);

    QMenu* menu = menuBar()->addMenu(tr("Management"));
    QAction* tableManagementAction = menu->addAction(tr("Tables Management"));

    connect(tableManagementAction, &QAction::triggered, this, &diplomski_obj1::showTableManagement);
}

diplomski_obj1::~diplomski_obj1()
{
    if (tableManagementWidget != nullptr)
        delete tableManagementWidget;
}

void diplomski_obj1::showTableManagement()
{
    if (tableManagementWidget == nullptr) {
        tableManagementWidget = new TableManagement(this);  
    }

    setCentralWidget(tableManagementWidget);  
    tableManagementWidget->show();
}
