#include "Cart.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QSpacerItem>

Cart::Cart(int userId, QWidget* parent)
    : QWidget(parent), userId(userId), totalPrice(0.0) 
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    QWidget* cartWidget = new QWidget(this);
    cartLayout = new QVBoxLayout(cartWidget);

    scrollArea->setWidget(cartWidget);
    mainLayout->addWidget(scrollArea);

    totalPriceLabel = new QLabel("Total Price: $0.00", this); 
    mainLayout->addWidget(totalPriceLabel);

    clearCartButton = new QPushButton("Clear Cart", this);
    clearCartButton->setStyleSheet("background-color: #e74c3c; color: white; padding: 10px;");
    connect(clearCartButton, &QPushButton::clicked, this, &Cart::handleClearCart);
    mainLayout->addWidget(clearCartButton);

    placeOrderButton = new QPushButton("Place Order", this);
    placeOrderButton->setStyleSheet("background-color: #2ecc71; color: white; padding: 10px;");
    connect(placeOrderButton, &QPushButton::clicked, this, &Cart::handlePlaceOrder);
    mainLayout->addWidget(placeOrderButton);

    cartLayout->setContentsMargins(10, 10, 10, 10);
    cartLayout->setSpacing(15);

    setLayout(mainLayout);

    loadCartItems();
}

Cart::~Cart() {}

size_t Cart::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void Cart::loadCartItems() {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/cart_items/" + std::to_string(userId);
        std::string responseStr;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(QString::fromStdString(responseStr).toUtf8());
            QJsonObject responseObject = responseDoc.object();

            if (responseObject.contains("items")) {
                cartItems = responseObject["items"].toArray();
                updateCart(cartItems);
                updateTotalPrice(cartItems);

                bool cartIsEmpty = cartItems.isEmpty();
                clearCartButton->setEnabled(!cartIsEmpty);
                placeOrderButton->setEnabled(!cartIsEmpty);

            }
            else {
                QMessageBox::information(this, "Cart", "No items in the cart.");
            }
        }
        else {
            QMessageBox::warning(this, "Error", "Failed to load cart items.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

bool Cart::checkCartStatus() {
    QJsonObject jsonPayload;
    jsonPayload["user_id"] = userId;
    QJsonDocument jsonDoc(jsonPayload);
    std::string jsonString = jsonDoc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    std::string responseBuffer;

    if (curl) {
        std::string url = "http://127.0.0.1:5000/check_cart";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            QMessageBox::warning(this, "Error", "Failed to check cart status.");
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return false;
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
        return false;
    }

    QJsonDocument responseDoc = QJsonDocument::fromJson(QByteArray::fromStdString(responseBuffer));
    if (!responseDoc.isObject()) {
        QMessageBox::warning(this, "Error", "Invalid response received when checking cart status.");
        return false;
    }

    QJsonObject responseObj = responseDoc.object();
    if (responseObj.contains("cart_id") && responseObj.contains("status")) {
        QString status = responseObj["status"].toString();
        if (status == "active") {
            return true;  
        }
        else {
            QMessageBox::warning(this, "Error", "Cart is already not-active. Cannot place order.");
            return false;  
        }
    }
    else if (responseObj.contains("cart_id") && responseObj["cart_id"].toInt() == -1) {
        QMessageBox::warning(this, "Error", "No active cart found. Please create a cart first.");
        return false; 
    }
    else if (responseObj.contains("error")) {
        QString errorMsg = responseObj["error"].toString();
        QMessageBox::warning(this, "Error", errorMsg);
    }

    return false; 
}

void Cart::updateCart(const QJsonArray& cartItems) {
    QLayoutItem* item;
    while ((item = cartLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (const QJsonValue& value : cartItems) {
        QJsonObject obj = value.toObject();
        QString itemName = obj["item_name"].toString();
        int quantity = obj["quantity"].toInt();
        double price = obj["price"].toDouble() * quantity;

        cartLayout->addWidget(createCartItemWidget(itemName, quantity, price));
    }

    cartLayout->addStretch(); 
}

void Cart::updateTotalPrice(const QJsonArray& cartItems) {
    double newTotalPrice = 0.0;
    for (const QJsonValue& value : cartItems) {
        QJsonObject obj = value.toObject();
        double price = obj["price"].toDouble();
        int quantity = obj["quantity"].toInt();
        newTotalPrice += price * quantity;
    }
    totalPrice = newTotalPrice;
    totalPriceLabel->setText(QString("Total Price: $%1").arg(QString::number(totalPrice, 'f', 2)));
}

QWidget* Cart::createCartItemWidget(const QString& itemName, int quantity, double price) {
    QWidget* itemWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(itemWidget);

    QLabel* nameLabel = new QLabel(itemName, this);
    QLabel* priceLabel = new QLabel(QString("Price: $%1").arg(QString::number(price, 'f', 2)), this);
    QLabel* quantityLabel = new QLabel(QString("Quantity: %1").arg(QString::number(quantity)), this);

    nameLabel->setStyleSheet("QLabel { color: black; }");
    priceLabel->setStyleSheet("QLabel { color: black; }");
    quantityLabel->setStyleSheet("QLabel { color: black; }");

    QPushButton* removeButton = new QPushButton("Remove item");
    removeButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; }");

    connect(removeButton, &QPushButton::clicked, this, [=]() {
        handleRemoveItem(itemName);
    });

    layout->addWidget(nameLabel);
    layout->addWidget(priceLabel);
    layout->addWidget(quantityLabel);
    layout->addWidget(removeButton);

    itemWidget->setStyleSheet("QWidget { border: 1px solid #ddd; padding: 10px; background-color: white; }");

    return itemWidget;
}

void Cart::handleRemoveItem(const QString& itemName) {
    QJsonObject json;
    json["user_id"] = userId;
    int itemId = getItemIdByName(itemName);
    json["item_id"] = itemId;

    QJsonDocument doc(json);
    std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/remove_cart_item";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QMessageBox::information(this, "Success", "Item removed from cart.");
        }
        else {
            QMessageBox::warning(this, "Error", "Failed to remove item.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    loadCartItems();
}

void Cart::handleClearCart() {
    QJsonObject json;
    json["user_id"] = userId;

    QJsonDocument doc(json);
    std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/clear_cart_items";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QMessageBox::information(this, "Success", "Cart cleared.");
        }
        else {
            QMessageBox::warning(this, "Error", "Failed to clear cart.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    loadCartItems(); 
}

void Cart::handleDeleteCart()
{
    QJsonObject json;
    json["user_id"] = userId;

    QJsonDocument doc(json);
    std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/clear_cart";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            QMessageBox::warning(this, "Error", "Failed to delete cart.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    loadCartItems();
}

QString Cart::getUserName() {
    std::string getUserInfoUrl = "http://127.0.0.1:5000/get_user_info/" + std::to_string(userId);
    std::string responseBuffer;

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, getUserInfoUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);  

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            QString errorMsg = QString("Failed to retrieve user information: %1").arg(curl_easy_strerror(res));
            QMessageBox::warning(this, "Error", errorMsg);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return QString();  
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
        return QString(); 
    }

    QJsonDocument responseDoc = QJsonDocument::fromJson(QByteArray::fromStdString(responseBuffer));
    if (responseDoc.isNull() || !responseDoc.isObject()) {
        QMessageBox::warning(this, "Error", "Invalid response received when fetching user information.");
        return QString();  
    }

    QJsonObject responseObj = responseDoc.object();
    if (responseObj.contains("full_name")) {
        return responseObj["full_name"].toString(); 
    }
    else if (responseObj.contains("error")) {
        QString errorMsg = responseObj["error"].toString();
        QMessageBox::warning(this, "Error", errorMsg);
    }
    else {
        QMessageBox::warning(this, "Error", "User name not found in the response.");
    }

    return QString(); 
}


void Cart::handlePlaceOrder() {
    if (cartItems.isEmpty()) {
        QMessageBox::warning(this, "Error", "Your cart is empty. Add items before placing an order.");
        return;
    }

    QJsonObject orderData;
    orderData["user_id"] = userId;
    orderData["customer_name"] = getUserName();
    orderData["role_id"] = 5; 

    QJsonArray orderItemsArray;
    for (const QJsonValue& value : cartItems) {
        QJsonObject cartItem = value.toObject();
        QJsonObject orderItem;
        orderItem["item_id"] = cartItem["item_id"].toInt();
        orderItem["quantity"] = cartItem["quantity"].toInt();
        orderItemsArray.append(orderItem);
    }
    orderData["order_items"] = orderItemsArray;

    QJsonDocument orderDoc(orderData);
    QString orderString = orderDoc.toJson(QJsonDocument::Compact);

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/place_order";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, orderString.toUtf8().constData());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string responseBuffer;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(QByteArray::fromStdString(responseBuffer));
            if (!responseDoc.isObject()) {
                QMessageBox::warning(this, "Error", "Invalid response from the server.");
            }
            else {
                QJsonObject responseObj = responseDoc.object(); 

                if (responseObj["status"].toString() == "success") {
                    QMessageBox::information(this, "Success", "Order placed successfully.");

                    handleClearCart();
                }
                else {
                    QString errorMsg = responseObj["message"].toString();
                    QMessageBox::warning(this, "Order Failed", QString("Error message: %1").arg(errorMsg));
                }
            }
        }
        else {
            QString errorMsg = QString("CURL request failed: %1").arg(curl_easy_strerror(res));
            QMessageBox::warning(this, "Error", errorMsg);
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

int Cart::getItemIdByName(const QString& itemName) {
    CURL* curl;
    CURLcode res;
    std::string response;
    int itemId = -1;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_item_id";

        QJsonObject json;
        json["item_name"] = itemName;
        QJsonDocument jsonDoc(json);
        std::string jsonPayload = jsonDoc.toJson(QJsonDocument::Compact).toStdString();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            QMessageBox::critical(this, "cURL Error", QString("cURL failed: ").arg(curl_easy_strerror(res)));
        }
        else {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(response).toUtf8());
            QJsonObject obj = doc.object();
            if (obj["status"] == "success") {
                itemId = obj["item_id"].toInt();
            }
            else {
                QMessageBox::critical(this, "Error", QString("Error: ").arg(obj["message"].toString()));
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return itemId; 
}