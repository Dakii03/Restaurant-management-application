#include "menu.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
#include <sqlite3.h>
#include <QDoubleValidator>
#include <QTabWidget>

MenuManagement::MenuManagement(QString username)
{
    mainLayout = new QVBoxLayout(this);
    currentUsername = username;

    QLabel* titleLabel = new QLabel("Menu Items", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    mainLayout->addWidget(titleLabel);

    tabWidget = new QTabWidget(this);

    foodListWidget = new QListWidget(this);
    drinkListWidget = new QListWidget(this);

    tabWidget->addTab(foodListWidget, "Food");
    tabWidget->addTab(drinkListWidget, "Drink");

    mainLayout->addWidget(tabWidget);

    // Add Button Layout  
    buttonLayout = new QHBoxLayout;
 
    populateMenuFromDatabase();
    
    modifyMenu();

    setLayout(mainLayout);
}

MenuManagement::~MenuManagement() {}

void MenuManagement::updateMenu(const QJsonArray& menuItems)
{
    foodListWidget->clear();
    drinkListWidget->clear();

    for (const QJsonValue& value : menuItems) {
        QJsonObject obj = value.toObject();
        QString itemName = obj["item_name"].toString();
        QString itemDescription = obj["description"].toString();
        double itemPrice = obj["price"].toDouble();
        int itemQuantity = obj["quantity"].toInt(); 
        QString type = obj["type"].toString();

        QString menuItem = QString("%1\nDescription: %2\nPrice: $%3\nQuantity: %4")
            .arg(itemName)
            .arg(itemDescription)
            .arg(itemPrice, 0, 'f', 2)
            .arg(itemQuantity); 

        QListWidgetItem* listItem = new QListWidgetItem(menuItem);
        listItem->setSizeHint(QSize(0, 80));

        if (type == "food") {
            foodListWidget->addItem(listItem);
        }
        else if (type == "drink") {
            drinkListWidget->addItem(listItem);
        }
    }
}

void MenuManagement::populateMenuFromDatabase()
{
    CURL* curl;
    CURLcode res;
    std::string responseStr;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/staff_menu_items";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(responseStr).toUtf8());
            QJsonObject responseObject = doc.object();

            if (responseObject.contains("items")) {
                QJsonArray menuItems = responseObject["items"].toArray();
                updateMenu(menuItems);
            }
            else {
                QMessageBox::warning(this, "Error", "Failed to fetch menu items.");
            }
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to perform the request.");
        }

        curl_easy_cleanup(curl);
    }
}

void MenuManagement::addMenuItem(const char* itemName, const char* description, double price, int quantity, const char* type) {
    CURL* curl;
    CURLcode res;
    std::string responseStr;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/add_menu_item";
        std::string payload = "{\"item_name\":\"" + std::string(itemName) + "\"," +
            "\"description\":\"" + std::string(description) + "\"," +
            "\"price\":" + std::to_string(price) + "," +
            "\"quantity\":" + std::to_string(quantity) + "," +
            "\"type\":\"" + std::string(type) + "\"}";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(responseStr).toUtf8());
            QJsonObject responseObject = doc.object();

            if (responseObject.contains("status") && responseObject["status"].toString() == "success") {
                QMessageBox::information(this, "Success", "Menu item added successfully.");
            }
            else {
                QMessageBox::critical(this, "Error", responseObject["message"].toString());
            }
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to perform the request.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

void MenuManagement::removeSelectedItem(const char* itemName, const char* type) {
    CURL* curl;
    CURLcode res;
    std::string responseStr;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/remove_menu_item";
        std::string payload = "{\"item_name\":\"" + std::string(itemName) + "\","
            "\"type\":\"" + std::string(type) + "\"}";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(responseStr).toUtf8());
            QJsonObject responseObject = doc.object();

            if (responseObject.contains("status") && responseObject["status"].toString() == "success") {
                QMessageBox::information(this, "Success", "Menu item removed successfully.");
            }
            else {
                QMessageBox::critical(this, "Error", responseObject["message"].toString());
            }
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to perform the request.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}


void MenuManagement::modifyMenu()
{
    if (getRoleByUsername(currentUsername) == "administrator" || getRoleByUsername(currentUsername) == "manager") {
        newItemLineEdit = new QLineEdit(this);
        newItemLineEdit->setPlaceholderText("Enter new menu item name");
        buttonLayout->addWidget(newItemLineEdit);

        newDescriptionLineEdit = new QLineEdit(this);
        newDescriptionLineEdit->setPlaceholderText("Enter item description");
        buttonLayout->addWidget(newDescriptionLineEdit);

        newPriceLineEdit = new QLineEdit(this);
        newPriceLineEdit->setPlaceholderText("Enter item price");
        QDoubleValidator* priceValidator = new QDoubleValidator(0.0, 10000.0, 2, this);
        newPriceLineEdit->setValidator(priceValidator);
        buttonLayout->addWidget(newPriceLineEdit);

        // New input field for quantity
        newQuantityLineEdit = new QLineEdit(this);
        newQuantityLineEdit->setPlaceholderText("Enter item quantity");
        QIntValidator* quantityValidator = new QIntValidator(0, 10000, this);
        newQuantityLineEdit->setValidator(quantityValidator);
        buttonLayout->addWidget(newQuantityLineEdit);

        addButton = new QPushButton("Add Menu Item", this);
        buttonLayout->addWidget(addButton);

        removeButton = new QPushButton("Remove Selected Item", this);
        buttonLayout->addWidget(removeButton);

        mainLayout->addLayout(buttonLayout);

        connect(addButton, &QPushButton::clicked, this, [this] {
            QString itemName = newItemLineEdit->text();
            QString description = newDescriptionLineEdit->text();
            QString priceText = newPriceLineEdit->text();
            QString quantityText = newQuantityLineEdit->text();
            QString type = (tabWidget->currentIndex() == 0) ? "food" : "drink";

            if (!itemName.isEmpty() && !description.isEmpty() && !priceText.isEmpty() && !quantityText.isEmpty()) {
                bool ok;
                double price = priceText.toDouble(&ok);
                int quantity = quantityText.toInt();

                if (ok) {
                    addMenuItem(itemName.toUtf8().constData(), description.toUtf8().constData(), price, quantity, type.toUtf8().constData());
 
                    newItemLineEdit->clear();
                    newDescriptionLineEdit->clear();
                    newPriceLineEdit->clear();
                    newQuantityLineEdit->clear();
                    populateMenuFromDatabase();   
                }
                else {
                    QMessageBox::warning(this, "Input Error", "Please enter valid price and quantity.");
                }
            }
            else {
                QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
            }
            });

        connect(removeButton, &QPushButton::clicked, this, [this] {
            QListWidget* currentListWidget = (tabWidget->currentIndex() == 0) ? foodListWidget : drinkListWidget;
            QListWidgetItem* selectedItem = currentListWidget->currentItem();

            if (selectedItem) {
                QString itemToRemove = selectedItem->text().split('\n').first().trimmed();
                QString type = (tabWidget->currentIndex() == 0) ? "food" : "drink";

                removeSelectedItem(itemToRemove.toUtf8().constData(), type.toUtf8().constData());
                delete currentListWidget->takeItem(currentListWidget->row(selectedItem));
            }
            else {
                QMessageBox::warning(this, "Selection Error", "Please select an item to remove.");
            }
        });
    }
}


QString MenuManagement::getRoleByUsername(const QString& username)
{
    CURL* curl;
    CURLcode res;
    std::string responseStr;
    QString roleName;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_user_role";
        std::string payload = "{\"username\": \"" + username.toStdString() + "\"}";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(responseStr).toUtf8());
            QJsonObject responseObject = doc.object();

            if (responseObject.contains("role_name")) {
                roleName = responseObject["role_name"].toString();
            }
            else if (responseObject.contains("error")) {
                QMessageBox::warning(this, "Error", responseObject["error"].toString());
            }
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to perform the request.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    return roleName;
}

size_t MenuManagement::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}