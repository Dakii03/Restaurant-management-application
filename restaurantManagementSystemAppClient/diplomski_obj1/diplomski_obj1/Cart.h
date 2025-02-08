#ifndef CART_H
#define CART_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QJsonArray>
#include <QComboBox>
#include <curl/curl.h>

class Cart : public QWidget {
    Q_OBJECT

public:
    explicit Cart(int userId, QWidget* parent = nullptr);
    ~Cart();

    void loadCartItems(); 
    void updateCart(const QJsonArray& cartItems); 
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    void handleClearCart();
    void handleDeleteCart();
    QString getUserName();
    bool checkCartStatus();
private:
    int userId;
    double totalPrice;
    QJsonArray cartItems;

    QVBoxLayout* cartLayout;
    QWidget* createCartItemWidget(const QString& itemName, int quantity, double price);
    QLabel* totalPriceLabel;
    QPushButton* placeOrderButton;
    QPushButton* clearCartButton;

private slots:
    void handleRemoveItem(const QString& itemName);
    int getItemIdByName(const QString& itemName);
    void updateTotalPrice(const QJsonArray& cartItems);
    void handlePlaceOrder();
    //void handleUpdateQuantity(const QString& itemName, int newQuantity);
};

#endif
