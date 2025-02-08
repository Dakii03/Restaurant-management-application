#include "Order.h"
#include "sqlite3.h"
#include <iostream>
#include <QMessageBox>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QTabWidget>

std::vector<int> parseOccupiedTables(const std::string& response) {
    std::vector<int> tables;

    size_t start = response.find('[');
    size_t end = response.find(']');

    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string array = response.substr(start + 1, end - start - 1);

        size_t pos = 0;
        std::string token;
        while ((pos = array.find(',')) != std::string::npos) {
            token = array.substr(0, pos);
            tables.push_back(std::stoi(token));
            array.erase(0, pos + 1);
        }

        // Push the last table number
        if (!array.empty()) {
            tables.push_back(std::stoi(array));
        }
    }

    return tables;
}



Order::Order(QWidget* parent)
    : QWidget(parent), currentTableNumber(-1), selectedButton(nullptr)
{
    mainLayout = new QVBoxLayout(this);

    tableLabel = new QLabel("Select a Table", this);
    mainLayout->addWidget(tableLabel);

    tableGridLayout = new QGridLayout();
    mainLayout->addLayout(tableGridLayout);

    loadOccupiedTables();

    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    foodListWidget = new QListWidget(this);
    drinkListWidget = new QListWidget(this);

    foodListWidget->setSelectionMode(QAbstractItemView::MultiSelection);
    drinkListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    tabWidget->addTab(foodListWidget, "Foods");
    tabWidget->addTab(drinkListWidget, "Drinks");

    loadMenuItems();

    placeOrderButton = new QPushButton("Place Order", this);
    mainLayout->addWidget(placeOrderButton);

    connect(placeOrderButton, &QPushButton::clicked, this, &Order::placeOrder);

    setLayout(mainLayout);
}

Order::~Order() {}

size_t Order::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void Order::loadOccupiedTables()
{
    CURL* curl;
    CURLcode res;
    std::string response_string;
    std::string url = "http://127.0.0.1:5000/get_occupied_tables";  

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return;
        }

        curl_easy_cleanup(curl);
    }

    std::vector<int> occupied_tables = parseOccupiedTables(response_string);

    int row = 0;
    int col = 0;
    int buttonCount = 0;
    const int maxButtons = 10;
    const int maxCols = 5;

    for (int tableNumber : occupied_tables) {
        QPushButton* tableButton = new QPushButton(QString("Table %1").arg(tableNumber), this);
        tableButton->setFixedSize(80, 80);
        tableButton->setStyleSheet("background-color: green; font-weight: bold;");  

        connect(tableButton, &QPushButton::clicked, this, [=]() {
            selectTableButton(tableButton, tableNumber);
        });

        tableGridLayout->addWidget(tableButton, row, col);
        col++;
        buttonCount++;
        if (col >= maxCols || buttonCount >= maxButtons) {
            col = 0;
            row++;
        }
    }
}

void Order::selectTableButton(QPushButton* button, int tableNumber)
{
    if (selectedButton != nullptr) {
        selectedButton->setStyleSheet("background-color: green; font-weight: bold;");
    }

    button->setStyleSheet("background-color: gray; font-weight: bold;");
    selectedButton = button;
    currentTableNumber = tableNumber;
    
    tableLabel->setText(QString("Selected Table: %1").arg(tableNumber));
}

void Order::loadMenuItems()
{
    std::vector<MenuItem> foodItems = fetchMenuItemsFromDatabase("food");
    std::vector<MenuItem> drinkItems = fetchMenuItemsFromDatabase("drink");

    populateMenuList(foodListWidget, foodItems);
    populateMenuList(drinkListWidget, drinkItems);
}

std::vector<MenuItem> Order::fetchMenuItemsFromDatabase(const std::string& type)
{
    std::vector<MenuItem> items;

    CURL* curl = curl_easy_init();
    if (!curl) {
        QMessageBox::critical(nullptr, "CURL Error", "Failed to initialize CURL.");
        return items;
    }

    std::string url = "http://127.0.0.1:5000/order_menu_items?type=" + type;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    std::string response_data;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Order::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        QString errorMsg = QString("CURL request failed: %1").arg(curl_easy_strerror(res));
        QMessageBox::critical(nullptr, "CURL Error", errorMsg);
        curl_easy_cleanup(curl);
        return items;
    }

    curl_easy_cleanup(curl);

    //QMessageBox::information(nullptr, "Raw JSON Response", QString::fromStdString(response_data));

    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(response_data));
    if (!doc.isObject()) {
        QMessageBox::critical(nullptr, "JSON Error", "Invalid JSON response received.");
        return items;
    }

    QJsonObject jsonObj = doc.object();
    if (!jsonObj.contains("items")) {
        QMessageBox::critical(nullptr, "JSON Error", "No 'items' field in JSON response.");
        return items;
    }

    QJsonArray jsonArray = jsonObj["items"].toArray();
    for (const QJsonValue& value : jsonArray) {
        QJsonObject obj = value.toObject();

        if (obj.contains("item_name") && obj.contains("price") && obj.contains("quantity") && obj.contains("item_id")) {
            MenuItem item;
            item.id = obj["item_id"].toInt();
            item.name = obj["item_name"].toString().toStdString();
            item.price = obj["price"].toDouble();
            item.quantity = obj["quantity"].toInt();

            items.push_back(item);
        }
        else {
            QMessageBox::warning(nullptr, "Parsing Error", "One or more fields missing in the JSON response.");
        }
    }

    return items;
}

void Order::populateMenuList(QListWidget* listWidget, const std::vector<MenuItem>& items)
{
    listWidget->clear();

    for (const auto& item : items) {
        std::string itemName = item.name;
        double price = item.price;
        int availableQuantity = item.quantity;
        int itemId = item.id;  

        QWidget* itemWidget = new QWidget(listWidget);
        QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);

        QCheckBox* itemCheckbox = new QCheckBox(itemWidget);
        itemCheckbox->setText(QString::fromStdString(itemName) + " - $" + QString::number(price, 'f', 2));

        itemCheckbox->setProperty("item_id", itemId);

        QLabel* quantityLabel = new QLabel(QString("Available: %1").arg(availableQuantity), itemWidget);

        QSpinBox* quantityInput = new QSpinBox(itemWidget);
        quantityInput->setRange(0, availableQuantity);
        quantityInput->setValue(0);
        quantityInput->setFixedWidth(100);

        itemLayout->addWidget(itemCheckbox);
        itemLayout->addWidget(quantityLabel);
        itemLayout->addWidget(new QLabel("Order:"));
        itemLayout->addWidget(quantityInput);

        itemLayout->setContentsMargins(5, 2, 5, 2);
        itemWidget->setLayout(itemLayout);

        QListWidgetItem* listItem = new QListWidgetItem(listWidget);
        listWidget->addItem(listItem);
        listWidget->setItemWidget(listItem, itemWidget);
    }
}


void Order::placeOrder()
{
    if (currentTableNumber == -1) {
        QMessageBox::warning(this, "Order", "No table selected. Please select an occupied table.");
        return;
    }

    QList<QListWidgetItem*> selectedItems;

    for (int i = 0; i < foodListWidget->count(); ++i) {
        QListWidgetItem* item = foodListWidget->item(i);
        QWidget* itemWidget = foodListWidget->itemWidget(item);
        if (!itemWidget) continue;

        QCheckBox* itemCheckbox = itemWidget->findChild<QCheckBox*>();
        if (itemCheckbox && itemCheckbox->isChecked()) {
            selectedItems.append(item);
        }
    }

    for (int i = 0; i < drinkListWidget->count(); ++i) {
        QListWidgetItem* item = drinkListWidget->item(i);
        QWidget* itemWidget = drinkListWidget->itemWidget(item);
        if (!itemWidget) continue;

        QCheckBox* itemCheckbox = itemWidget->findChild<QCheckBox*>();
        if (itemCheckbox && itemCheckbox->isChecked()) {
            selectedItems.append(item);
        }
    }

    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Order", "No items selected. Please select at least one food or drink item.");
        return;
    }

    QJsonArray itemsArray;
    for (QListWidgetItem* item : selectedItems) {
        QWidget* itemWidget = foodListWidget->itemWidget(item);
        if (!itemWidget) {
            itemWidget = drinkListWidget->itemWidget(item);
        }

        if (itemWidget) {
            QCheckBox* itemCheckbox = itemWidget->findChild<QCheckBox*>();

            if (itemCheckbox) {
                QString itemText = itemCheckbox->text();
                QString itemName = itemText.split(" - $").first();
                double itemPrice = itemText.split(" - $").last().toDouble();
                int itemId = itemCheckbox->property("item_id").toInt();
                QSpinBox* quantityInput = itemWidget->findChild<QSpinBox*>();
                int quantity = quantityInput ? quantityInput->value() : 0;

                if (quantity > 0) {
                    QJsonObject itemObj;
                    itemObj["id"] = itemId;
                    itemObj["name"] = itemName;
                    itemObj["price"] = itemPrice;
                    itemObj["quantity"] = quantity;

                    itemsArray.append(itemObj);
                }
            }
        }
    }

    if (itemsArray.isEmpty()) {
        QMessageBox::warning(this, "Order", "Please select at least one item with a valid quantity.");
        return;
    }

    QJsonObject postData;
    postData["table_number"] = currentTableNumber;
    postData["items"] = itemsArray;

    QJsonDocument jsonDoc(postData);
    std::string jsonString = jsonDoc.toJson().toStdString();

    CURL* curl = curl_easy_init();
    if (!curl) {
        QMessageBox::critical(this, "CURL Error", "Failed to initialize CURL.");
        return;
    }

    std::string url = "http://127.0.0.1:5000/place_order_staff";
    std::string response_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Order::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        QString errorMsg = QString("CURL request failed: %1").arg(curl_easy_strerror(res));
        QMessageBox::critical(this, "CURL Error", errorMsg);
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);

    QJsonDocument responseDoc = QJsonDocument::fromJson(QByteArray::fromStdString(response_data));
    if (!responseDoc.isObject()) {
        QMessageBox::critical(this, "Response Error", "Invalid JSON response from server.");
        return;
    }

    QJsonObject responseObject = responseDoc.object();
    if (responseObject["status"] == "success") {
        double totalAmount = responseObject["total_amount"].toDouble();
        QString message = QString("Order placed successfully for Table %1.\nTotal Amount: $%2")
            .arg(currentTableNumber)
            .arg(totalAmount, 0, 'f', 2);
        QMessageBox::information(this, "Order Placed", message);
    }
    else {
        QString errorMsg = responseObject["message"].toString();
        QMessageBox::warning(this, "Order Failed", errorMsg);
    }

    loadMenuItems();
    emit orderPlaced(currentTableNumber);
}


void Order::refreshTables()
{
    QLayoutItem* item;
    while ((item = tableGridLayout->takeAt(0)) != nullptr) {
        delete item->widget(); 
        delete item;
    }

    loadOccupiedTables();

    selectedButton = nullptr;
    currentTableNumber = -1;
    tableLabel->setText("Select a Table");
}

