#include "KitchenOrderManager.h"

KitchenOrderManager::KitchenOrderManager(int role_id, const QString& username, QWidget* parent)
    : QWidget(parent) {

    setupUI();
    fetchOrders();
}

void KitchenOrderManager::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel("Kitchen Staff - Orders", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: white; margin-bottom: 30px;");

    orderTable = new QTableWidget(this);
    orderTable->setColumnCount(4);
    orderTable->setHorizontalHeaderLabels({ "Order ID", "Customer", "Items", "Status" });

    orderTable->setStyleSheet(
        "QHeaderView::section { background-color: #444444; color: white; padding: 10px; border: 1px solid #dddddd; font-size: 18px; font-weight: bold; }"
        "QTableWidget { gridline-color: #dddddd; background-color: #ffffff; alternate-background-color: #f5f5f5; color: #333333; font-size: 16px; }"
        "QTableWidget::item { padding: 12px; color: #333333; font-size: 16px; font-family: Arial, sans-serif; }"
        "QTableWidget::item:selected { background-color: #d1e7ff; color: #000000; }" 
        "QTableWidget::item:hover { background-color: #c8dafc; }" 
    );

    orderTable->setAlternatingRowColors(true);
    orderTable->setSelectionBehavior(QAbstractItemView::SelectRows);  
    orderTable->setSelectionMode(QAbstractItemView::SingleSelection); 
    orderTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderTable->horizontalHeader()->setStretchLastSection(true);
    orderTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    orderTable->verticalHeader()->setDefaultSectionSize(50);  

   
    orderTable->setColumnWidth(0, 100);  // Order ID
    orderTable->setColumnWidth(1, 150);  // Customer
    orderTable->setColumnWidth(2, 250);  // Items
    orderTable->setColumnWidth(3, 150);  // Status

    QPushButton* acceptButton = new QPushButton("Accept Order", this);
    QPushButton* rejectButton = new QPushButton("Decline Order", this);
    QPushButton* completeButton = new QPushButton("Complete Order", this);

    acceptButton->setStyleSheet("background-color: #4CAF50; color: white; font-size: 18px; font-weight: bold; padding: 12px;");
    rejectButton->setStyleSheet("background-color: #F44336; color: white; font-size: 18px; font-weight: bold; padding: 12px;");
    completeButton->setStyleSheet("background-color: #2196F3; color: white; font-size: 18px; font-weight: bold; padding: 12px;");

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);
    buttonLayout->addWidget(completeButton);

    layout->addWidget(titleLabel);
    layout->addWidget(orderTable);
    layout->addLayout(buttonLayout);

    connect(acceptButton, &QPushButton::clicked, this, &KitchenOrderManager::acceptOrder);
    connect(rejectButton, &QPushButton::clicked, this, &KitchenOrderManager::rejectOrder);
    connect(completeButton, &QPushButton::clicked, this, &KitchenOrderManager::completeOrder);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &KitchenOrderManager::fetchOrders);
    timer->start(3000);

    fetchOrders();
}

void KitchenOrderManager::fetchOrders() {
    int selectedRow = -1;
    if (orderTable->currentRow() != -1) {
        selectedRow = orderTable->currentRow(); 
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/fetch_customer_orders";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        std::string responseBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(QByteArray::fromStdString(responseBuffer));
            if (!responseDoc.isNull() && responseDoc.isArray()) {
                QJsonArray orderArray = responseDoc.array();
                orderTable->setRowCount(0);  
                int visibleRowCount = 0;  

                for (const QJsonValue& value : orderArray) {
                    QJsonObject order = value.toObject();
                    QString status = order["order_status"].toString();

                    if (status == "Completed" || status == "Declined") {
                        continue;
                    }

                    orderTable->insertRow(visibleRowCount);
                    orderTable->setItem(visibleRowCount, 0, new QTableWidgetItem(QString::number(order["order_id"].toInt())));
                    orderTable->setItem(visibleRowCount, 1, new QTableWidgetItem(order["customer_name"].toString()));
                    orderTable->setItem(visibleRowCount, 2, new QTableWidgetItem(order["order_details"].toString()));
                    orderTable->setItem(visibleRowCount, 3, new QTableWidgetItem(status));

                    visibleRowCount++;
                }

                if (selectedRow != -1 && selectedRow < visibleRowCount) {
                    orderTable->selectRow(selectedRow);
                }
            }
            else {
                QMessageBox::warning(this, "Error", "Failed to parse the server response.");
            }
        }
        else {
            QMessageBox::critical(this, "Error", QString("Failed to fetch orders: %1").arg(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }
}

void KitchenOrderManager::acceptOrder() {
    int selectedRow = orderTable->currentRow();
    if (selectedRow >= 0) {
        int orderId = orderTable->item(selectedRow, 0)->text().toInt();
        //QMessageBox::warning(this, "order_id", QString::number(orderId));
        updateOrderStatus(orderId, "Accepted");
    }
    else {
        QMessageBox::warning(this, "No Selection", "Please select an order to accept.");
    }
}

void KitchenOrderManager::updateOrderStatus(int orderId, const QString& newStatus) {
    CURL* curl = curl_easy_init();
    if (curl) {
        QJsonObject json;
        json["order_id"] = orderId;
        json["status"] = newStatus;

        QJsonDocument doc(json);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

        //QMessageBox::information(this, "Debug - Payload", QString::fromStdString(payload));

        std::string url = "http://127.0.0.1:5000/update_order_status";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string responseBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(QByteArray::fromStdString(responseBuffer));

            //QMessageBox::information(this, "Debug - Server Response", QString::fromStdString(responseBuffer));

            if (!responseDoc.isNull() && responseDoc.isObject()) {
                QJsonObject responseObj = responseDoc.object();
                if (responseObj["status"].toString() == "success") {
                    //QMessageBox::information(this, "Success", "Order status updated successfully.");
                    fetchOrders();  
                }
                else {
                    QString errorMessage = responseObj["message"].toString();
                    QMessageBox::warning(this, "Server Error", QString("Error: %1").arg(errorMessage));
                }
            }
            else {
                QMessageBox::warning(this, "Error", "Invalid response received from the server.");
            }
        }
        else {
            QString errorMsg = QString("Failed to update order status: %1").arg(curl_easy_strerror(res));
            QMessageBox::critical(this, "CURL Error", errorMsg);
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }
}

void KitchenOrderManager::rejectOrder() {
    int selectedRow = orderTable->currentRow();
    if (selectedRow >= 0) {
        int orderId = orderTable->item(selectedRow, 0)->text().toInt();
        updateOrderStatus(orderId, "Declined");
    }
    else {
        QMessageBox::warning(this, "No Selection", "Please select an order to reject.");
    }
}

void KitchenOrderManager::completeOrder() {
    int selectedRow = orderTable->currentRow();
    if (selectedRow < 0) {
        QMessageBox::warning(this, "Error", "Please select an order to complete.");
        return;
    }

    QString orderStatus = orderTable->item(selectedRow, 3)->text();

    if (orderStatus == "Accepted") {
        int orderId = orderTable->item(selectedRow, 0)->text().toInt();
        updateOrderStatus(orderId, "Completed");
    }
    else {
        QMessageBox::warning(this, "Error", "You can only complete an accepted order.");
    }
}


size_t KitchenOrderManager::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}