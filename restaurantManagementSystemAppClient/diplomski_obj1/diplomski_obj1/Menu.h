#ifndef MENU_H 
#define MENU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QComboBox>
#include <QTabWidget>
#include "sqlite3.h"
#include <curl/curl.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class MenuManagement : public QWidget
{
    Q_OBJECT

public:
    explicit MenuManagement(QString username);
    ~MenuManagement();
    void populateMenuFromDatabase();

private slots:
    void addMenuItem(const char* itemName, const char* description, double price, int quantity, const char* type);
    void removeSelectedItem(const char*, const char*);
    void modifyMenu();
    //void onTabChanged(int index); 
    QString getRoleByUsername(const QString& username);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    void updateMenu(const QJsonArray& menuItems);

private:
    QVBoxLayout* mainLayout;
    QLineEdit* newDescriptionLineEdit;
    QLineEdit* newPriceLineEdit;
    QLineEdit* newItemLineEdit;
    QLineEdit* newQuantityLineEdit;
    QPushButton* addButton;
    QPushButton* removeButton;
    QTabWidget* tabWidget;          
    QListWidget* currentListWidget; 
    QListWidget* foodListWidget;
    QListWidget* drinkListWidget;
    QComboBox* typeComboBox;
    QString role;
    QHBoxLayout* buttonLayout;
    QString currentUsername;

    
};

#endif 
