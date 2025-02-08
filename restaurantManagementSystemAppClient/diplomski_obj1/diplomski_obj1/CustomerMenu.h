#ifndef CUSTOMER_MENU_H
#define CUSTOMER_MENU_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <curl/curl.h>
#include <string>
#include <vector>

class CustomerMenu : public QWidget
{
    Q_OBJECT

public:
    explicit CustomerMenu(int userId = 0, QWidget* parent = nullptr);
    ~CustomerMenu();

signals:
    void addedToCart();

private slots:
    void showFoodItems();
    void showDrinkItems();
    void handleAddToCart(const QString& itemName, int quantity, int userId);
    int getItemIdByName(const QString& itemName);
   
private:
    void setupLayout();
    void fetchMenuItems();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    QListWidget* categoryListWidget;
    QStackedWidget* itemStackWidget;

    QWidget* createMenuItemWidget(const QString& name, const QString& description, const QString& price, const QString& imageUrl);

    std::string menuItemsData;
    int userId;
    int cartId;
};

#endif 
