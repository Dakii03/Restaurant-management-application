#ifndef KITCHENORDERMANAGER_H
#define KITCHENORDERMANAGER_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <curl/curl.h>
#include <QHeaderView>
#include <QTimer>

class KitchenOrderManager : public QWidget {
    Q_OBJECT

public:
    explicit KitchenOrderManager(int role_id = 0, const QString& username = "", QWidget* parent = nullptr);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
private:
    QTableWidget* orderTable;
    
    void setupUI();
    void fetchOrders();
    void acceptOrder();
    void rejectOrder();
    void completeOrder();
    void updateOrderStatus(int orderId, const QString& newStatus);
};

#endif 
