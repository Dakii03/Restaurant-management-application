#include "billing.h"
#include <sqlite3.h>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <iostream>
#include <QFileDialog>

Billing::Billing(QWidget* parent)
    : QWidget(parent), totalAmount(0), discount(0), currentTableId(-1), currentOrderId(-1)
{
    setupUI();
    loadOccupiedTables();
}

Billing::~Billing() {}

void Billing::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QGroupBox* tableSelectionGroup = new QGroupBox("Table Selection", this);
    QVBoxLayout* tableSelectionLayout = new QVBoxLayout(tableSelectionGroup);
    tableComboBox = new QComboBox(this);
    connect(tableComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Billing::handleTableSelection);
    
    tableSelectionLayout->addWidget(new QLabel("Select Table for Checkout:", this));
    tableSelectionLayout->addWidget(tableComboBox);

    tableSelectionGroup->setLayout(tableSelectionLayout);
    mainLayout->addWidget(tableSelectionGroup);

    QGroupBox* tableDetailsGroup = new QGroupBox("Table Details", this);
    QVBoxLayout* tableDetailsLayout = new QVBoxLayout(tableDetailsGroup);

    selectedTableLabel = new QLabel("No table selected.", this);
    selectedTableLabel->setWordWrap(true);
    selectedTableLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    tableDetailsLayout->addWidget(selectedTableLabel);

    tableDetailsGroup->setLayout(tableDetailsLayout);
    mainLayout->addWidget(tableDetailsGroup);

    QGroupBox* billingGroup = new QGroupBox("Billing Information", this);
    QFormLayout* billingLayout = new QFormLayout(billingGroup);

    totalLabel = new QLabel("Total: $0.00", this);
    totalLabel->setStyleSheet("font-size: 16px; color: green;");
    billingLayout->addRow(new QLabel("Total Amount:", this), totalLabel);

    discountSpinBox = new QDoubleSpinBox(this);
    discountSpinBox->setRange(0, 100);
    discountSpinBox->setSuffix("%");
    discountSpinBox->setValue(0);
    billingLayout->addRow(new QLabel("Discount (%):", this), discountSpinBox);

    applyDiscountButton = new QPushButton("Apply Discount", this);
    connect(applyDiscountButton, &QPushButton::clicked, this, &Billing::applyDiscount);
    billingLayout->addWidget(applyDiscountButton);

    paymentMethodComboBox = new QComboBox(this);
    paymentMethodComboBox->addItems({ "Cash", "Credit/Debit Card" });
    billingLayout->addRow(new QLabel("Payment Method:", this), paymentMethodComboBox);

    payButton = new QPushButton("Finalize Payment", this);
    payButton->setStyleSheet("background-color: #5cb85c; color: white; font-size: 16px;");
    connect(payButton, &QPushButton::clicked, this, &Billing::finalizePayment);
    billingLayout->addWidget(payButton);

    billingGroup->setLayout(billingLayout);
    mainLayout->addWidget(billingGroup);

    setLayout(mainLayout);
}

void Billing::loadOccupiedTables()
{
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_occupied_tables_billing";
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            qDebug() << "cURL error:" << curl_easy_strerror(res);
        }
        else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(response).toUtf8());
            if (jsonResponse.isArray()) {
                QJsonArray tablesArray = jsonResponse.array();

                tableComboBox->clear();

                if (tablesArray.isEmpty()) {
                    selectedTableLabel->setText("No items found for the latest unpaid order.");
                    emit everythingPaid(); 
                }
                else {
                    for (const QJsonValue& value : tablesArray) {
                        QJsonObject tableObj = value.toObject();
                        int tableId = tableObj["table_id"].toInt();
                        int tableNumber = tableObj["table_number"].toInt();
                        tableComboBox->addItem(QString("Table %1").arg(tableNumber), tableId);
                    }
                }
            }
            else {
                qDebug() << "Invalid JSON response.";
            }
        }
        curl_easy_cleanup(curl);
    }
    else {
        qDebug() << "Failed to initialize cURL.";
    }
}

void Billing::handleTableSelection(int index)
{
    if (index == -1 || tableComboBox->count() == 0) {
        selectedTableLabel->setText("No items found for the latest unpaid order.");
        return;
    }
    currentTableId = tableComboBox->itemData(index).toInt();
    loadOrderForTable(currentTableId);
}


void Billing::loadOrderForTable(int tableId)
{
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_latest_order/" + std::to_string(tableId);
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            qDebug() << "cURL error:" << curl_easy_strerror(res);
        }
        else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(response).toUtf8());
            if (jsonResponse.isArray()) {
                QJsonArray orderArray = jsonResponse.array();

                items.clear();
                selectedTableLabel->setText(QString("Orders for Table %1:").arg(tableComboBox->currentText()));

                QStringList orderDetails;
                totalAmount = 0;

                for (const QJsonValue& value : orderArray) {
                    QJsonObject orderObj = value.toObject();
                    QString itemName = orderObj["item_name"].toString();
                    double itemPrice = orderObj["price"].toDouble();
                    int quantity = orderObj["quantity"].toInt();
                    double totalItemPrice = itemPrice * quantity;

                    items[itemName] = totalItemPrice;
                    totalAmount += totalItemPrice;
                    orderDetails.append(QString("%1 x%2 = $%3").arg(itemName).arg(quantity).arg(totalItemPrice, 0, 'f', 2));
                }

                if (orderDetails.isEmpty()) {
                    selectedTableLabel->setText("No items found for the latest unpaid order.");
                }
                else {
                    selectedTableLabel->setText(orderDetails.join("\n"));
                }

                updateTotalAmount();
            }
            else {
                qDebug() << "Invalid JSON response.";
            }
        }
        curl_easy_cleanup(curl);
    }
    else {
        qDebug() << "Failed to initialize cURL.";
    }
}


void Billing::applyDiscount()
{
    double discountValue = discountSpinBox->value();
    double discountAmount = totalAmount * (discountValue / 100);
    double discountedTotal = totalAmount - discountAmount;

    totalAmount = discountedTotal;
    totalLabel->setText(QString("Total after %1% discount: $%2").arg(discountValue).arg(discountedTotal, 0, 'f', 2));
    totalLabel->setStyleSheet("color: #17a2b8; font-size: 16px;");
}

void Billing::finalizePayment()
{
    if (currentTableId == -1) {
        QMessageBox::warning(this, "No Table Selected", "Please select a table to finalize the payment.");
        return;
    }
    if (savePaymentToDatabase()) {
        emit paymentFinalized(currentTableId);
        discountSpinBox->setValue(0);
        loadOccupiedTables(); 
    }
}



void Billing::updateTotalAmount()
{
    totalLabel->setText(QString("Total: $%1").arg(totalAmount, 0, 'f', 2));
}

bool Billing::savePaymentToDatabase()
{
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/process_payment";

        QString selectedPaymentMethod = paymentMethodComboBox->currentText().toLower();
        std::string paymentMethod = selectedPaymentMethod.toStdString(); 

        QJsonObject json;
        json["table_id"] = currentTableId;
        json["total_amount"] = totalAmount;
        json["payment_method"] = QString::fromStdString(paymentMethod);

        QJsonDocument jsonDoc(json);
        std::string payload = jsonDoc.toJson(QJsonDocument::Compact).toStdString();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.size());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string responseString;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            QMessageBox::warning(this, "Error", QString("cURL error: %1").arg(curl_easy_strerror(res)));
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            return false;
        }
        else {
            QString rawResponse = QString::fromStdString(responseString);
            //QMessageBox::information(this, "Raw response content: ", "" + rawResponse);

            QJsonDocument jsonResponse = QJsonDocument::fromJson(rawResponse.toUtf8());
            if (!jsonResponse.isObject()) {
                QMessageBox::warning(this, "Error", "Invalid server response format.");
                curl_slist_free_all(headers);
                curl_easy_cleanup(curl);
                return false;
            }

            QJsonObject responseObject = jsonResponse.object();
            QString status = responseObject["status"].toString();
            QString message = responseObject["message"].toString();

            if (status == "success") {
                QMessageBox::information(this, "Payment Successful", "The payment has been successfully processed!");
                loadOccupiedTables();
            }
            else {
                QMessageBox::warning(this, "Payment Failed", QString("Error from server: %1").arg(message));
            }
        }
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return true;
    }
    else {
        QMessageBox::warning(this, "Error", "Failed to initialize cURL");
        return false;
    }
}


size_t Billing::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append((char*)contents, totalSize);
    return totalSize;
}