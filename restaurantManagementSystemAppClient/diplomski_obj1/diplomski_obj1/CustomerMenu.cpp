#include "CustomerMenu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <iostream>
#include <QMessageBox>

CustomerMenu::CustomerMenu(int userId, QWidget* parent)
    : QWidget(parent), menuItemsData(""), userId(userId)
{
    setupLayout();
    fetchMenuItems();
}

CustomerMenu::~CustomerMenu() {}

void CustomerMenu::setupLayout()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    categoryListWidget = new QListWidget(this);
    categoryListWidget->addItem("Food");
    categoryListWidget->addItem("Drinks");

    categoryListWidget->setFixedWidth(150);
    categoryListWidget->setStyleSheet("font-size: 16px; padding: 10px;");
    connect(categoryListWidget, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
        if (item->text() == "Food") {
            showFoodItems();
        }
        else if (item->text() == "Drinks") {
            showDrinkItems();
        }
    });

    itemStackWidget = new QStackedWidget(this);
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(itemStackWidget);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->addWidget(categoryListWidget);
    contentLayout->addWidget(scrollArea);  

    mainLayout->addLayout(contentLayout);
    setLayout(mainLayout);
}

void CustomerMenu::showFoodItems()
{
    itemStackWidget->setCurrentWidget(new QWidget(this));  

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(menuItemsData).toUtf8());
    QJsonArray itemsArray = doc.array();

    QWidget* foodWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(foodWidget);

    for (const QJsonValue& value : itemsArray) {
        QJsonObject obj = value.toObject();
        if (obj["type"].toString() == "food") {
            layout->addWidget(createMenuItemWidget(
                obj["item_name"].toString(),
                obj["description"].toString(),
                QString::number(obj["price"].toDouble()),
                obj["image_url"].toString())
            );  
        }
    }

    layout->addStretch();  

    itemStackWidget->addWidget(foodWidget);
    itemStackWidget->setCurrentWidget(foodWidget);
}

void CustomerMenu::showDrinkItems()
{
    itemStackWidget->setCurrentWidget(new QWidget(this));  

    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(menuItemsData).toUtf8());
    QJsonArray itemsArray = doc.array();

    QWidget* drinksWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(drinksWidget);

    for (const QJsonValue& value : itemsArray) {
        QJsonObject obj = value.toObject();
        if (obj["type"].toString() == "drink") {
            layout->addWidget(createMenuItemWidget(
                obj["item_name"].toString(),
                obj["description"].toString(),
                QString::number(obj["price"].toDouble()),
                obj["image_url"].toString())
            );  
        }
    }

    layout->addStretch(); 

    itemStackWidget->addWidget(drinksWidget);
    itemStackWidget->setCurrentWidget(drinksWidget);
}

QWidget* CustomerMenu::createMenuItemWidget(const QString& name, const QString& description, const QString& price, const QString& imageUrl)
{
    QWidget* itemWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(itemWidget);

    QLabel* imageLabel = new QLabel(this);
    QPixmap pixmap;
    if (!imageUrl.isEmpty() && pixmap.load(imageUrl)) {
        imageLabel->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio));
        imageLabel->setAlignment(Qt::AlignCenter);
    }
    else {
        imageLabel->setText("No image available");
        imageLabel->setStyleSheet("color: #FF0000;"); 
    }

    QLabel* nameLabel = new QLabel("<b>" + name + "</b>", this);
    nameLabel->setStyleSheet(
        "font-size: 18px;"
        "color: #333333;" 
    );

    QLabel* descLabel = new QLabel(description, this);
    descLabel->setStyleSheet(
        "font-size: 14px;"
        "color: #666666;"  
    );

    QLabel* priceLabel = new QLabel("<b>Price: $" + price + "</b>", this);
    priceLabel->setStyleSheet(
        "font-size: 16px;"
        "color: #FF4500;" 
    );

    QComboBox* quantityComboBox = new QComboBox(this);
    for (int i = 1; i <= 10; ++i) {
        quantityComboBox->addItem(QString::number(i));  
    }

    quantityComboBox->setStyleSheet(
        "color: black;"              
        "background-color: white;"   
        "font-size: 14px;"           
        "padding: 5px;"              
    );

    QPushButton* addToCartButton = new QPushButton("Add to Cart", this);
    addToCartButton->setStyleSheet(
        "background-color: #4CAF50;"  
        "color: white;"               
        "padding: 10px 15px;"
        "border-radius: 5px;"
    );

    connect(addToCartButton, &QPushButton::clicked, this, [=]() {
        int quantity = quantityComboBox->currentText().toInt();
        handleAddToCart(name, quantity, userId);  
    });

    QHBoxLayout* actionLayout = new QHBoxLayout();
    QLabel* quantityLabel = new QLabel("Quantity: ", this);
    quantityLabel->setStyleSheet("color: black; font-size: 16px;");
    quantityComboBox->setStyleSheet("background-color: #333333");
    actionLayout->addWidget(quantityLabel);
    actionLayout->addWidget(quantityComboBox);
    actionLayout->addWidget(addToCartButton);

    layout->setSpacing(10);
    layout->addWidget(imageLabel);
    layout->addWidget(nameLabel);
    layout->addWidget(descLabel);
    layout->addWidget(priceLabel);
    layout->addLayout(actionLayout);
    layout->setContentsMargins(10, 10, 10, 10); 

    itemWidget->setStyleSheet("background-color: #ffffff;");

    return itemWidget;
}

void CustomerMenu::handleAddToCart(const QString& itemName, int quantity, int userId) {
    if (userId == -1) {
        QMessageBox::critical(this, "Authentication Error", QString("User not authenticated!"));
        return;
    }

    QJsonObject checkCartJson;
    checkCartJson["user_id"] = userId;

    QJsonDocument checkCartDoc(checkCartJson);
    std::string checkCartPayload = checkCartDoc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string checkCartUrl = "http://127.0.0.1:5000/check_cart";
        std::string responseStr;

        curl_easy_setopt(curl, CURLOPT_URL, checkCartUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, checkCartPayload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to check cart: " << curl_easy_strerror(res) << std::endl;
            QMessageBox::warning(this, "Cart Error", "Failed to check cart.");
            curl_easy_cleanup(curl);
            return;
        }

        QJsonDocument responseDoc = QJsonDocument::fromJson(QString::fromStdString(responseStr).toUtf8());
        QJsonObject responseObject = responseDoc.object();
        int cartId = responseObject["cart_id"].toInt(-1);
        std::string status = responseObject["status"].toString().toStdString();
        //QMessageBox::information(this, "cart_id: ", QString::number(cartId));

        if (cartId == -1 || status == "not-active") {
            QJsonObject createCartJson;
            createCartJson["user_id"] = userId;

            QJsonDocument createCartDoc(createCartJson);
            std::string createCartPayload = createCartDoc.toJson(QJsonDocument::Compact).toStdString();

            std::string createCartUrl = "http://127.0.0.1:5000/create_cart";
            curl_easy_setopt(curl, CURLOPT_URL, createCartUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, createCartPayload.c_str());

            responseStr.clear();  

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                QMessageBox::warning(this, "Cart Error", "Failed to create cart.");
                curl_easy_cleanup(curl);  
                return;
            }

            responseDoc = QJsonDocument::fromJson(QString::fromStdString(responseStr).toUtf8());
            responseObject = responseDoc.object();
            cartId = responseObject["cart_id"].toInt(-1); 
            //QMessageBox::information(this, "Cart Created", QString::number(cartId));  // For testing

            if (cartId == -1) {
                QMessageBox::critical(this, "Error", "Failed to create or retrieve cart ID.");
                curl_easy_cleanup(curl);
                return;
            }
        }

        QJsonObject json;
        json["cart_id"] = cartId;
        QJsonObject item;
        item["item_id"] = getItemIdByName(itemName);  
        item["quantity"] = quantity;
        QJsonArray itemsArray;
        itemsArray.append(item);
        json["items"] = itemsArray;

        QJsonDocument jsonDoc(json);
        std::string jsonPayload = jsonDoc.toJson(QJsonDocument::Compact).toStdString();

        std::string addToCartUrl = "http://127.0.0.1:5000/add_to_cart";
        curl_easy_setopt(curl, CURLOPT_URL, addToCartUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to add item to cart: " << curl_easy_strerror(res) << std::endl;
            QMessageBox::warning(this, "Cart Error", "Failed to add item to cart.");
        }
        else {
            QMessageBox::information(this, "Cart", "Item added to cart successfully.");
        }

        emit addedToCart();
        curl_easy_cleanup(curl);
    }
}

int CustomerMenu::getItemIdByName(const QString& itemName)
{
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
            std::cerr << "cURL failed: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(response).toUtf8());
            QJsonObject obj = doc.object();
            if (obj["status"] == "success") {
                itemId = obj["item_id"].toInt();
                //QMessageBox::information(this, "TEST", "" + QString::number(itemId));
            }
            else {
                std::cerr << "Error: " << obj["message"].toString().toStdString() << std::endl;
            }
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return itemId;
}

void CustomerMenu::fetchMenuItems()
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/menu_items";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &menuItemsData);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;

        curl_easy_cleanup(curl);
    }
    showFoodItems();
}

size_t CustomerMenu::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

