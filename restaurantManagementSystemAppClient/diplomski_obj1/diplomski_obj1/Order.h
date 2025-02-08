#ifndef ORDER_H
#define ORDER_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include "table_management.h"
#include <vector>
#include <string>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <curl/curl.h>
#include <QSpinBox>
#include <QCheckBox>

struct MenuItem {
    int id;
    std::string name;
    double price;
    int quantity;
};

class Order : public QWidget
{
    Q_OBJECT

public:
    explicit Order(QWidget* parent = nullptr);
    ~Order();
    void refreshTables();       

private slots:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);
    void loadOccupiedTables();  
    void loadMenuItems();       
    void selectTableButton(QPushButton* button, int tableNumber);  
    void placeOrder();         


signals:
    void orderPlaced(int tableNumber);

private:
    void populateMenuList(QListWidget* listWidget, const std::vector<MenuItem>& items); 
    std::vector<MenuItem> fetchMenuItemsFromDatabase(const std::string& type);          

    QLabel* tableLabel;           
    QVBoxLayout* mainLayout;      
    QGridLayout* tableGridLayout; 
    QTabWidget* tabWidget;        
    QListWidget* foodListWidget;  
    QListWidget* drinkListWidget;
    QPushButton* placeOrderButton; 
    QSpinBox* quantitySpinBox;

    int currentTableNumber;      
    QPushButton* selectedButton;  
    QMap<QString, double> currentOrderItems;
};

#endif
