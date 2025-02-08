#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_diplomski_obj1.h"
#include "table_management.h"  

class diplomski_obj1 : public QMainWindow
{
    Q_OBJECT

public:
    diplomski_obj1(QWidget* parent = nullptr);
    ~diplomski_obj1();

private slots:
    void showTableManagement();  

private:
    Ui::diplomski_obj1Class ui;
    TableManagement* tableManagementWidget;  
};
