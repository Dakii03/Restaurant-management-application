#include "table_management.h"
#include <QDate>
#include <iostream>

TableManagement::TableManagement(QWidget* parent)
    : QWidget(parent), currentTableNumber(-1)
{
    mainLayout = new QVBoxLayout(this);
    //initializeTablesInDatabase();

    createTableOverview();

    mainLayout->addSpacing(20);

    QHBoxLayout* legendLayout = new QHBoxLayout();

    QLabel* freeLabel = new QLabel();
    freeLabel->setText("Free");
    freeLabel->setStyleSheet("background-color: green; color: white; padding: 8px; border-radius: 5px;");
    freeLabel->setFont(QFont("Arial", 12));
    freeLabel->setAlignment(Qt::AlignCenter);
    freeLabel->setPixmap(QIcon("check.png").pixmap(40, 40));

    QLabel* reservedLabel = new QLabel();
    reservedLabel->setText("Reserved");
    reservedLabel->setStyleSheet("background-color: yellow; color: black; padding: 8px; border-radius: 5px;");
    reservedLabel->setFont(QFont("Arial", 12));
    reservedLabel->setAlignment(Qt::AlignCenter);
    reservedLabel->setPixmap(QIcon("lock.png").pixmap(40, 40));

    QLabel* occupiedLabel = new QLabel();
    occupiedLabel->setText("Occupied");
    occupiedLabel->setStyleSheet("background-color: red; color: white; padding: 8px; border-radius: 5px;");
    occupiedLabel->setFont(QFont("Arial", 12));
    occupiedLabel->setAlignment(Qt::AlignCenter);
    occupiedLabel->setPixmap(QIcon("user.png").pixmap(40, 40));

    legendLayout->addWidget(freeLabel);
    legendLayout->addWidget(reservedLabel);
    legendLayout->addWidget(occupiedLabel);

    mainLayout->addLayout(legendLayout);

    mainLayout->addSpacing(30);

    tableDetailsLabel = new QLabel("Select a table to view details", this);
    tableDetailsLabel->setFont(QFont("Arial", 14, QFont::Bold));
    tableDetailsLabel->setStyleSheet("color: #495057; padding: 10px;");
    mainLayout->addWidget(tableDetailsLabel, 0, Qt::AlignCenter);

    releaseButton = new QPushButton("Release Table", this);
    releaseButton->setEnabled(false);  
    releaseButton->setFixedSize(160, 45); 
    releaseButton->setStyleSheet(
        "background-color: #6c757d; color: white; font-size: 16px; "
        "border-radius: 8px; padding: 8px; border: none;"
        "transition: background-color 0.3s ease;"
    );
    releaseButton->setCursor(Qt::PointingHandCursor);  

    connect(releaseButton, &QPushButton::pressed, this, [this] {
        releaseButton->setStyleSheet("background-color: #dc3545; color: white; font-size: 16px; border-radius: 8px;");
    });

    connect(releaseButton, &QPushButton::released, this, [this] {
        releaseButton->setStyleSheet(
            "background-color: #6c757d; color: white; font-size: 16px; border-radius: 8px; padding: 8px; border: none;"
        );
    });

    mainLayout->addWidget(releaseButton, 0, Qt::AlignCenter);

    connect(releaseButton, &QPushButton::clicked, this, &TableManagement::releaseTable);

    setLayout(mainLayout);
}

TableManagement::~TableManagement() {}

void TableManagement::createTableOverview() {
    tableOverviewGroupBox = new QGroupBox("Table Overview", this);
    tableOverviewGroupBox->setFont(QFont("Arial", 16, QFont::Bold));
    tableOverviewGroupBox->setStyleSheet("padding: 15px; border: 2px solid #ced4da; border-radius: 8px;");

    QGridLayout* gridLayout = new QGridLayout;

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/create_table_overview");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(readBuffer).toUtf8());
            QJsonArray tableArray = jsonResponse.array();

            for (int i = 0; i < tableArray.size(); ++i) {
                QJsonObject tableObj = tableArray[i].toObject();
                int tableNumber = tableObj["table_number"].toInt();

                tables[i] = new QPushButton(QString("Table %1").arg(tableNumber), this);
                tables[i]->setFixedSize(110, 60);

                tables[i]->setStyleSheet(
                    "background-color: green; color: white; font-size: 16px; "
                    "border-radius: 10px; padding: 8px;"
                );

                tables[i]->setToolTip(QString("Table %1").arg(tableNumber));
                connect(tables[i], &QPushButton::clicked, [this, tableNumber] { handleTableClick(tableNumber); });

                gridLayout->addWidget(tables[i], i / 5, i % 5);  
            }
        }
        else {
            QMessageBox::warning(this, "Error", "Failed to load tables.");
        }
    }

    tableOverviewGroupBox->setLayout(gridLayout);
    mainLayout->addWidget(tableOverviewGroupBox);
}

size_t TableManagement::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

void TableManagement::releaseAllTables() {
    for (int i = 0; i < 10; ++i) {
        updateTableStatus(i + 1, "Free");

        updateTableOccupiedStatusInDatabase(i + 1, false);
    }

    tableDetailsLabel->setText("All tables are now Free");

    emit tableStatusChanged();
}


void TableManagement::refreshTables()
{
    int* reservedTables = checkReservedTables();
    int* occupiedTables = checkOccupiedTables();

    for (int i = 0; i < 10; ++i)
    {
        tables[i]->setStyleSheet(
            "background-color: green; color: white; font-size: 16px; "
            "border-radius: 10px; padding: 8px;"
        );
        tables[i]->setToolTip("This table is Free");
        tables[i]->setEnabled(true);
    }

    if (reservedTables != nullptr)
    {
        for (int i = 0; reservedTables[i] != -1; ++i)  
        {
            int tableIndex = reservedTables[i];
            if (tableIndex >= 1 && tableIndex < 11) 
            {
                tables[tableIndex - 1]->setStyleSheet(
                    "background-color: yellow; color: black; font-size: 16px; "
                    "border-radius: 10px; padding: 8px;"
                );
                tables[tableIndex - 1]->setToolTip("This table is Reserved");
                tables[tableIndex - 1]->setEnabled(false);
            }
        }
    }

    if (occupiedTables != nullptr)
    {
        for (int i = 0; occupiedTables[i] != -1; ++i)  
        {
            int tableIndex = occupiedTables[i];
            if (tableIndex >= 1 && tableIndex < 11)
            {
                tables[tableIndex - 1]->setStyleSheet(
                    "background-color: red; color: white; font-size: 16px; "
                    "border-radius: 10px; padding: 8px;"
                );
                tables[tableIndex - 1]->setToolTip("This table is Occupied");
            }
        }
    }

    delete[] reservedTables;
    delete[] occupiedTables;
}



void TableManagement::handleTableClick(int tableNumber){
    currentTableNumber = tableNumber;

    tableDetailsLabel->setText(QString("Details of Table %1:\nStatus: Occupied").arg(tableNumber));
    updateTableStatus(tableNumber, "Occupied");
    updateTableOccupiedStatusInDatabase(tableNumber, true);
    releaseButton->setEnabled(true);

    emit tableStatusChanged();
}

void TableManagement::releaseTable()
{
    if (currentTableNumber < 1 || currentTableNumber > 10) return;

    updateTableStatus(currentTableNumber, "Free");
    tableDetailsLabel->setText(QString("Table %1 is now Free").arg(currentTableNumber));
    updateTableOccupiedStatusInDatabase(currentTableNumber, false);

    emit tableStatusChanged();

    currentTableNumber = -1;
    releaseButton->setEnabled(false);
}

void TableManagement::updateTableStatus(int tableNumber, const QString& status)
{
    if (tableNumber < 1 || tableNumber > 10) return;

    QPushButton* selectedTable = tables[tableNumber - 1];

    if (status == "Free") {
        selectedTable->setStyleSheet("background-color: green; color: white;");
        selectedTable->setToolTip("This table is Free");
    }
    else if (status == "Occupied") {
        selectedTable->setStyleSheet("background-color: red; color: white;");
        selectedTable->setToolTip("This table is Occupied");
    }
    else if (status == "Reserved") {
        selectedTable->setStyleSheet("background-color: yellow; color: black;");
        selectedTable->setToolTip("This table is Reserved");
    }
}


//void TableManagement::initializeTablesInDatabase()
//{
//    sqlite3* db;
//    sqlite3_stmt* stmt;
//
//    int rc = sqlite3_open("C:\\Users\\Lenovo\\source\\repos\\diplomski_obj1\\x64\\Release\\restaurant.db", &db);
//    if (rc != SQLITE_OK) {
//        QMessageBox::warning(this, "Database error", QString("Cannot open database: %1").arg(sqlite3_errmsg(db)));
//        return;
//    }
//    const char* checkQuery = "SELECT COUNT(*) FROM tables";
//    rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, nullptr);
//    if (rc != SQLITE_OK) {
//        QMessageBox::warning(this, "Database error", QString("Failed to query database: %1").arg(sqlite3_errmsg(db)));
//        sqlite3_close(db);
//        return;
//    }
//
//    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) == 0) {
//        const char* insertQuery = "INSERT INTO tables (table_number, capacity, is_occupied) VALUES (?, ?, ?)";
//        sqlite3_finalize(stmt);  
//
//        rc = sqlite3_prepare_v2(db, insertQuery, -1, &stmt, nullptr);
//        if (rc != SQLITE_OK) {
//            QMessageBox::warning(this, "Database error", QString("Failed to prepare table insertion: %1").arg(sqlite3_errmsg(db)));
//            sqlite3_close(db);
//            return;
//        }
//
//        for (int i = 1; i <= 10; i++) {
//            sqlite3_bind_int(stmt, 1, i);  // Table number
//            sqlite3_bind_int(stmt, 2, 4);  
//            sqlite3_bind_int(stmt, 3, 0);  
//
//            rc = sqlite3_step(stmt);
//            if (rc != SQLITE_DONE) {
//                QMessageBox::warning(this, "Database error", QString("Failed to insert table: %1").arg(sqlite3_errmsg(db)));
//                break;
//            }
//
//            sqlite3_reset(stmt);  
//        }
//    }
//
//    sqlite3_finalize(stmt);
//    sqlite3_close(db);
//}


void TableManagement::updateTableOccupiedStatusInDatabase(int tableNumber, bool isOccupied) {
    CURL* curl;
    CURLcode res;

    std::string jsonPayload = "{\"table_number\": " + std::to_string(tableNumber) +
        ", \"is_occupied\": " + (isOccupied ? "1" : "0") + "}";

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/update_table_occupied_status");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to send cURL request: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}

void TableManagement::handlePaymentFinalization(int tableNumber) {
    if (tableNumber >= 1 && tableNumber <= 10) {
        updateTableStatus(tableNumber, "Free");
        updateTableOccupiedStatusInDatabase(tableNumber, false);

        tableDetailsLabel->setText(QString("Table %1 is now Free").arg(tableNumber));
    }
}

int* TableManagement::checkReservedTables() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    int* reservedTables = new int[11];  
    int i = 0;

    for (int j = 0; j < 11; ++j) {
        reservedTables[j] = -1;
    }

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/check_reserved_tables");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to send cURL request: " << curl_easy_strerror(res) << std::endl;
            delete[] reservedTables;
            return nullptr;
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(readBuffer).toUtf8());
        QJsonArray tableArray = jsonResponse.array();

        for (const QJsonValue& value : tableArray) {
            if (i < 10) {  
                reservedTables[i] = value.toInt();
                ++i;
            }
        }

        curl_easy_cleanup(curl);
    }
    else {
        delete[] reservedTables;
        return nullptr;
    }

    return reservedTables; 
}

int* TableManagement::checkOccupiedTables() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    int* occupiedTables = new int[11];  
    int i = 0;

    for (int j = 0; j < 11; ++j) {
        occupiedTables[j] = -1;
    }

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/check_occupied_tables");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Failed to send cURL request: " << curl_easy_strerror(res) << std::endl;
            delete[] occupiedTables;
            return nullptr;
        }

        QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(readBuffer).toUtf8());
        QJsonArray tableArray = jsonResponse.array();

        for (const QJsonValue& value : tableArray) {
            if (i < 10) { 
                occupiedTables[i] = value.toInt();
                ++i;
            }
        }

        curl_easy_cleanup(curl);
    }
    else {
        delete[] occupiedTables;
        return nullptr;
    }

    return occupiedTables; 
}