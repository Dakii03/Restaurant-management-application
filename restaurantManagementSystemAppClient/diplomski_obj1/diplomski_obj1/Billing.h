#ifndef BILLING_H
#define BILLING_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QString>
#include <QMap>
#include <QGroupBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <curl/curl.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <QTimer>

class Billing : public QWidget
{
    Q_OBJECT

public:
    explicit Billing(QWidget* parent = nullptr);
    ~Billing();

    void loadOccupiedTables();

private slots:
    void handleTableSelection(int index);
    void applyDiscount();
    void finalizePayment();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);
    //void generateAndShowQRCode(int orderId);
    //static size_t WriteImageCallback(void* contents, size_t size, size_t nmemb, void* userp);
    //bool verifyQRCodePayment(int orderId);
    //int getCurrentOrderId(int tableId);
signals:
    void paymentFinalized(int tableNumber);
    void everythingPaid();

private:
    QComboBox* tableComboBox;        
    QComboBox* paymentMethodComboBox;
    QLabel* selectedTableLabel;      
    QLabel* totalLabel;              
    QDoubleSpinBox* discountSpinBox; 
    QPushButton* calculateButton;    
    QPushButton* applyDiscountButton;
    QPushButton* payButton;          

    double totalAmount;              
    double discount;                 
    int currentTableId;              
    int currentOrderId;
    QMap<QString, double> items;  

    void setupUI();

    void loadOrderForTable(int tableId);

    void updateTotalAmount();

    bool savePaymentToDatabase();
};

#endif 
