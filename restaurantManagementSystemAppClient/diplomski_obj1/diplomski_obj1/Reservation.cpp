#include "Reservation.h"
#include <QDebug>

Reservation::Reservation(QWidget* parent)
    : QWidget(parent)
{
    if (!openDatabase()) {
        QMessageBox::critical(this, "Database Error", "Failed to connect to the database.");
        return;
    }

    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &Reservation::checkForPastReservations);
    updateTimer->start(3000);

    removePastReservations();

    setWindowTitle("Table Reservation");

    mainLayout = new QGridLayout(this);
    mainLayout->setSpacing(10);

    QFont labelFont("Arial", 12);

    nameLabel = new QLabel("Name:");
    nameLabel->setFont(labelFont);
    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Enter name");
    nameEdit->setStyleSheet("border-radius: 5px; padding: 5px;");

    dateLabel = new QLabel("Date:");
    dateLabel->setFont(labelFont);
    dateEdit = new QDateEdit();
    dateEdit->setDate(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setStyleSheet("font-size: 14px;");

    timeLabel = new QLabel("Time:");
    timeLabel->setFont(labelFont);
    timeEdit = new QTimeEdit();
    timeEdit->setTime(QTime::currentTime());
    timeEdit->setDisplayFormat("HH:mm");
    timeEdit->setStyleSheet("font-size: 14px;");

    guestsLabel = new QLabel("Number of Guests:");
    guestsLabel->setFont(labelFont);
    guestsSpinBox = new QSpinBox();
    guestsSpinBox->setRange(1, 4);
    guestsSpinBox->setStyleSheet("font-size: 14px;");

    tableLabel = new QLabel("Table:");
    tableLabel->setFont(labelFont);
    tableComboBox = new QComboBox();
    tableComboBox->setStyleSheet("font-size: 14px;");

    reserveButton = new QPushButton("Reserve");
    reserveButton->setStyleSheet("background-color: #4CAF50; color: white; font-size: 14px; padding: 10px; border: none; border-radius: 5px;");
    connect(reserveButton, &QPushButton::clicked, this, &Reservation::makeReservation);

    reservationsTable = new QTableWidget();
    reservationsTable->setColumnCount(6); 
    reservationsTable->setHorizontalHeaderLabels(QStringList() << "Reservation ID" << "Name" << "Table" << "Guests" << "Date & Time" << "Remove reservation");
    //reservationsTable->setSelectionBehavior(QAbstractItemView::SelectRows); // Make full rows selectable
    //reservationsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // Disable editing

    reservationsTable->horizontalHeader()->setStretchLastSection(true);
    reservationsTable->setStyleSheet("QTableWidget { background-color: #f9f9f9; font-size: 14px; }");

    int row = 0;
    mainLayout->addWidget(nameLabel, row, 0);
    mainLayout->addWidget(nameEdit, row++, 1);

    mainLayout->addWidget(dateLabel, row, 0);
    mainLayout->addWidget(dateEdit, row++, 1);

    mainLayout->addWidget(timeLabel, row, 0);
    mainLayout->addWidget(timeEdit, row++, 1);

    mainLayout->addWidget(guestsLabel, row, 0);
    mainLayout->addWidget(guestsSpinBox, row++, 1);

    mainLayout->addWidget(tableLabel, row, 0);
    mainLayout->addWidget(tableComboBox, row++, 1);

    mainLayout->addWidget(reserveButton, row, 0, 1, 2, Qt::AlignCenter);

    mainLayout->addWidget(reservationsTable, ++row, 0, 1, 2);

    setLayout(mainLayout);

    populateAvailableTables();

    loadReservationsFromDatabase();
}

Reservation::~Reservation()
{
}

bool Reservation::openDatabase()
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        QMessageBox::critical(this, "cURL error", "Failed to initialize cURL");
        return false;
    }

    std::string url = "http://127.0.0.1:5000/open_database";
    std::string response_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        qDebug() << "CURL request failed:" << curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_cleanup(curl);

    // Parse response from Flask
    QJsonDocument responseDoc = QJsonDocument::fromJson(QByteArray::fromStdString(response_data));
    if (!responseDoc.isObject()) {
        qDebug() << "Invalid JSON response";
        return false;
    }

    QJsonObject responseObject = responseDoc.object();
    if (responseObject["status"] == "success") {
        qDebug() << "Database opened successfully on the server.";
        return true;
    }
    else {
        qDebug() << "Failed to open database on the server:" << responseObject["message"].toString();
        return false;
    }
}


void Reservation::loadReservationsFromDatabase()
{
    reservationsTable->setRowCount(0); 

    CURL* curl = curl_easy_init();
    if (!curl) {
        qDebug() << "Failed to initialize cURL";
        return;
    }

    std::string url = "http://127.0.0.1:5000/get_reservations";
    std::string response_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        qDebug() << "CURL request failed:" << curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        return;
    }

    curl_easy_cleanup(curl);

    QJsonDocument jsonResponse = QJsonDocument::fromJson(QByteArray::fromStdString(response_data));
    if (!jsonResponse.isArray()) {
        qDebug() << "Invalid JSON response";
        return;
    }

    QJsonArray jsonArray = jsonResponse.array();
    int row = 0;

    reservationsTable->setColumnWidth(0, 100); // Reservation ID column
    reservationsTable->setColumnWidth(1, 100); // Name column
    reservationsTable->setColumnWidth(2, 100); // Table column
    reservationsTable->setColumnWidth(3, 100); // Guests column
    reservationsTable->setColumnWidth(5, 80);  // Delete button column 
    reservationsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);  // Date & Time

    for (const QJsonValue& value : jsonArray) {
        QJsonObject obj = value.toObject();

        reservationsTable->insertRow(row);

        int reservationId = obj["reservation_id"].toInt();
        QString name = obj["name"].toString();
        int tableId = obj["table_id"].toInt();
        int guests = obj["guest_count"].toInt();
        QString dateTime = obj["reservation_time"].toString();

        QTableWidgetItem* reservationIdItem = new QTableWidgetItem(QString::number(reservationId));
        reservationIdItem->setForeground(Qt::black);

        QTableWidgetItem* nameItem = new QTableWidgetItem(name);
        nameItem->setForeground(Qt::black);

        QTableWidgetItem* tableIdItem = new QTableWidgetItem(QString::number(tableId));
        tableIdItem->setForeground(Qt::black);

        QTableWidgetItem* guestsItem = new QTableWidgetItem(QString::number(guests));
        guestsItem->setForeground(Qt::black);

        QTableWidgetItem* dateTimeItem = new QTableWidgetItem(dateTime);
        dateTimeItem->setForeground(Qt::black);

        reservationsTable->setItem(row, 0, reservationIdItem);
        reservationsTable->setItem(row, 1, nameItem);
        reservationsTable->setItem(row, 2, tableIdItem);
        reservationsTable->setItem(row, 3, guestsItem);
        reservationsTable->setItem(row, 4, dateTimeItem);

        QPushButton* deleteButton = new QPushButton("Delete");
        deleteButton->setStyleSheet("background-color: red; color: white; font-size: 14px; padding: 5px;");
        reservationsTable->setCellWidget(row, 5, deleteButton);

        connect(deleteButton, &QPushButton::clicked, this, [this, reservationId]() {
            deleteReservation(reservationId);
        });

        row++;
    }
    emit reservationsLoaded();
}

void Reservation::checkForPastReservations(){
    removePastReservations();
    loadReservationsFromDatabase();
}

size_t Reservation::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s)
{
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    /*size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    }
    catch (std::bad_alloc& e) {
        return 0;
    }
    return newLength;*/
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void Reservation::populateAvailableTables()
{
    tableComboBox->clear();

    QDate selectedDate = dateEdit->date();
    QTime selectedTime = timeEdit->time();
    QString selectedDateTime = selectedDate.toString("yyyy-MM-dd") + " " + selectedTime.toString("HH:mm:ss");

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_available_tables";
        std::string jsonPayload = "{\"datetime\":\"" + selectedDateTime.toStdString() + "\"}";
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(response).toUtf8());
            if (jsonResponse.isArray()) {
                QJsonArray tableArray = jsonResponse.array();
                for (const QJsonValue& value : tableArray) {
                    int tableId = value.toInt();
                    tableComboBox->addItem(QString::number(tableId), tableId);
                }
            }
        }

        if (tableComboBox->count() == 0) {
            tableComboBox->addItem("No available tables", -1);
            tableComboBox->setDisabled(true);
        }
        else {
            tableComboBox->setDisabled(false);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}

void Reservation::makeReservation()
{
    std::string name = nameEdit->text().toStdString();
    QDate date = dateEdit->date();
    QTime time = timeEdit->time();
    QDateTime dateTime = QDateTime(date, time);
    int guests = guestsSpinBox->value();
    int tableId = tableComboBox->currentData().toInt();

    if (name.empty()) {
        QMessageBox::warning(this, "Input Error", "Please enter your name.");
        return;
    }

    if (tableId == -1) {
        QMessageBox::warning(this, "Table Error", "No tables available for the selected date and time.");
        return;
    }

    if (!checkReservationDate(dateTime)) {
        QMessageBox::warning(this, "Date Error", "Reservation date must be after current date.");
        return;
    }

    saveReservation(name, date, time, guests, tableId);
    loadReservationsFromDatabase();
    populateAvailableTables();
    nameEdit->setText("");
    nameEdit->setPlaceholderText("Enter name");
    dateEdit->setDate(QDate::currentDate());
    timeEdit->setTime(QTime::currentTime());
    guestsSpinBox->setValue(1);

}

void Reservation::saveReservation(const std::string& name, const QDate& date, const QTime& time, int guests, int tableId)
{
    QJsonObject json;
    json["name"] = QString::fromStdString(name);
    json["table_id"] = tableId;
    json["guest_count"] = guests;
    json["reservation_time"] = date.toString("yyyy-MM-dd") + " " + time.toString("HH:mm:ss");

    QJsonDocument doc(json);
    std::string jsonPayload = doc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/save_reservation";
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(response).toUtf8());
            if (jsonResponse.isObject()) {
                QJsonObject obj = jsonResponse.object();
                if (obj["status"] == "success") {
                    qDebug() << "Reservation saved successfully!";
                    QMessageBox::information(this, "Reservation Successful", "Your reservation has been saved.");
                }
                else {
                    QString errorMessage = obj["message"].toString();
                    qDebug() << "Error saving reservation:" << errorMessage;
                    QMessageBox::warning(this, "Reservation Failed", errorMessage);
                }
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}


void Reservation::removePastReservations()
{
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/remove_past_reservations";
        std::string jsonPayload = "{\"current_time\":\"" + currentDateTime.toStdString() + "\"}";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonPayload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }
        else {
            std::cout << "Request to remove past reservations sent successfully!" << std::endl;
        }

        // Cleanup
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}


bool Reservation::checkReservationDate(QDateTime& date) {
    if (date >= QDateTime::currentDateTime()) {
        return true;
    }

    return false;
}

void Reservation::deleteReservation(int reservationId)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete Reservation", "Are you sure you want to delete this reservation?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        CURL* curl = curl_easy_init();
        if (curl) {
            std::string url = "http://127.0.0.1:5000/delete_reservation/" + std::to_string(reservationId);

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

            std::string responseString;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                qDebug() << "cURL error:" << curl_easy_strerror(res);
            }
            else {
                QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(responseString).toUtf8());
                if (!jsonResponse.isObject()) {
                    QMessageBox::warning(this, "Error", "Invalid server response format.");
                }
                else {
                    QJsonObject responseObject = jsonResponse.object();
                    QString status = responseObject["status"].toString();
                    QString message = responseObject["message"].toString();

                    if (status == "success") {
                        qDebug() << "Reservation deleted successfully!";
                        QMessageBox::information(this, "Deleted", "The reservation has been deleted.");

                        loadReservationsFromDatabase();
                        populateAvailableTables();
                    }
                    else {
                        QMessageBox::warning(this, "Delete Failed", message);
                    }
                }
            }
            curl_easy_cleanup(curl);
        }
        else {
            QMessageBox::warning(this, "Error", "Failed to initialize cURL");
        }
    }
}
