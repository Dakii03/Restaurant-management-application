#include "Staff_management.h"
#include "sqlite3.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QFormLayout>
#include <QDateEdit>
#include <QSpacerItem>
#include <QCryptographicHash> 

StaffManagement::StaffManagement(QWidget* parent) :
    QWidget(parent),
    staffTable(new QTableWidget(this)),
    addButton(new QPushButton("Add Staff", this)),
    deleteButton(new QPushButton("Delete Staff", this)),
    departmentEdit(new QLineEdit(this)),
    positionComboBox(new QComboBox(this)),
    hireDateEdit(new QDateEdit(this)),
    userIdEdit(new QLineEdit(this)) 
{
    hireDateEdit->setCalendarPopup(true);
    hireDateEdit->setDisplayFormat("yyyy-MM-dd");

    QVBoxLayout* layout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout;

    userIdEdit->setPlaceholderText("Auto-assigned User ID");
    userIdEdit->setReadOnly(true);

    departmentEdit->setPlaceholderText("Auto-assigned Department");
    departmentEdit->setReadOnly(true);
    hireDateEdit->setToolTip("Select Hire Date");

    formLayout->addRow(new QLabel("User ID (Auto):"), userIdEdit);
    formLayout->addRow(new QLabel("Department:"), departmentEdit);
    formLayout->addRow(new QLabel("Position:"), positionComboBox);
    formLayout->addRow(new QLabel("Hire Date (YYYY-MM-DD):"), hireDateEdit);

    layout->addLayout(formLayout);
    layout->addWidget(staffTable);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    layout->addLayout(buttonLayout);

    staffTable->setColumnCount(4);
    staffTable->setHorizontalHeaderLabels({ "User ID", "Department", "Position", "Hire Date" });
    staffTable->horizontalHeader()->setStretchLastSection(true);
    staffTable->setAlternatingRowColors(true);

    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

    loadRolesFromDatabase();

    loadStaffFromDatabase();

    connect(addButton, &QPushButton::clicked, this, &StaffManagement::addStaff);
    connect(deleteButton, &QPushButton::clicked, this, &StaffManagement::deleteStaff);
}

StaffManagement::~StaffManagement() {}

void StaffManagement::loadStaffFromDatabase()
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/load_staff");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            QMessageBox::critical(this, "Error", "Failed to fetch staff data");
        }
        else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(readBuffer).toUtf8());
            QJsonArray staffArray = jsonResponse.array();

            staffTable->setRowCount(0);

            for (int i = 0; i < staffArray.size(); ++i) {
                QJsonObject staffObj = staffArray[i].toObject();
                int row = staffTable->rowCount();
                staffTable->insertRow(row);

                staffTable->setItem(row, 0, new QTableWidgetItem(QString::number(staffObj["user_id"].toInt())));
                staffTable->setItem(row, 1, new QTableWidgetItem(staffObj["department"].toString()));
                staffTable->setItem(row, 2, new QTableWidgetItem(staffObj["position"].toString()));
                staffTable->setItem(row, 3, new QTableWidgetItem(staffObj["hire_date"].toString()));
            }
        }
        curl_easy_cleanup(curl);
    }
}

void StaffManagement::loadRolesFromDatabase()
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/load_roles");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            QMessageBox::critical(this, "Error", "Failed to fetch roles data");
        }
        else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(readBuffer).toUtf8());
            QJsonArray rolesArray = jsonResponse.array();

            positionComboBox->clear();

            for (int i = 0; i < rolesArray.size(); ++i) {
                QJsonObject roleObj = rolesArray[i].toObject();
                QString role = roleObj["role_name"].toString();
                positionComboBox->addItem(role);
            }
        }
        curl_easy_cleanup(curl);
    }
}

void StaffManagement::getNextUserId()
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/get_next_user_id");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            QMessageBox::critical(this, "Error", "Failed to fetch next user ID");
        }
        else {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(readBuffer).toUtf8());
            QJsonObject jsonObj = jsonResponse.object();

            if (jsonObj.contains("next_user_id")) {
                int nextUserId = jsonObj["next_user_id"].toInt();
                userIdEdit->setText(QString::number(nextUserId));
            }
            else {
                QMessageBox::critical(this, "Error", "Invalid response from server");
            }
        }
        curl_easy_cleanup(curl);
    }
}


QString StaffManagement::hashPassword(const QString& password)
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

void StaffManagement::addStaffToDatabase(const QString& department, const QString& position, const QString& hireDate)
{
    getNextUserId();
    int userId = userIdEdit->text().toInt();

    QList<QString> permissions; 

    CURL* curl = curl_easy_init();
    if (curl) {
        QJsonObject jsonData;
        jsonData["user_id"] = userId;
        jsonData["department"] = department;
        jsonData["position"] = position;
        jsonData["hire_date"] = hireDate;

        QJsonDocument doc(jsonData);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();
        std::string url = "http://127.0.0.1:5000/add_staff";

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
            QJsonObject responseObject = responseDoc.object();

            if (!responseDoc.isNull() && responseObject["status"] == "success") {
                QMessageBox::information(this, "Success", "Staff data inserted successfully.");
            }
            else {
                QString errorMessage = responseObject["message"].toString();
                QMessageBox::critical(this, "Error", QString("Server Error: %1").arg(errorMessage));
            }
        }
        else {
            QMessageBox::critical(this, "Error", QString("Failed to insert staff data: %1").arg(curl_easy_strerror(res)));
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }

    loadStaffFromDatabase();
}


void StaffManagement::deleteStaffFromDatabase(int userId)
{
    QList<QString> permissions; 

    CURL* curl = curl_easy_init();
    if (curl) {
        QJsonObject jsonData;
        jsonData["user_id"] = userId;

        QJsonDocument doc(jsonData);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

        std::string url = "http://127.0.0.1:5000/delete_staff";
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
            QJsonObject responseObject = responseDoc.object();

            if (!responseDoc.isNull() && responseObject["status"] == "success") {
                QMessageBox::information(this, "Success", "Staff member deleted successfully.");
            }
            else {
                QString errorMessage = responseObject["message"].toString();
                QMessageBox::critical(this, "Error", QString("Server Error: %1").arg(errorMessage));
            }
        }
        else {
            QMessageBox::critical(this, "Error", QString("Failed to delete staff member: %1").arg(curl_easy_strerror(res)));
        }
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }
    loadStaffFromDatabase();  
}


void StaffManagement::addStaff() {
    QString position = positionComboBox->currentText();
    QString department;

    if (position == "administrator") {
        department = "Administration";
    }
    else if (position == "waiter") {
        department = "Front of House (FOH)";
    }
    else if (position == "kitchen_staff") {
        department = "Back of House (BOH)";
    }
    else if (position == "manager") {
        department = "General Management/Human Resources";
    }
    else {
        department = "Unassigned";
    }

    QString userId = userIdEdit->text();

    QString hireDate = hireDateEdit->date().toString("yyyy-MM-dd");

    int row = staffTable->rowCount();
    staffTable->insertRow(row);
    staffTable->setItem(row, 0, new QTableWidgetItem(userId));  
    staffTable->setItem(row, 1, new QTableWidgetItem(department));  
    staffTable->setItem(row, 2, new QTableWidgetItem(position));  
    staffTable->setItem(row, 3, new QTableWidgetItem(hireDate));  

    addStaffToDatabase(department, position, hireDate);

    userIdEdit->clear();
    departmentEdit->clear();  
    positionComboBox->setCurrentIndex(0);  
    hireDateEdit->setDate(QDate::currentDate());  
}

void StaffManagement::deleteStaff()
{
    QList<QTableWidgetItem*> selectedItems = staffTable->selectedItems();

    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select a staff member to delete");
        return;
    }
    int selectedRow = staffTable->currentRow();
    QTableWidgetItem* userIdItem = staffTable->item(selectedRow, 0); 
    int userId = userIdItem->text().toInt();

    if (userId == 0) {
        QMessageBox::warning(this, "Deletion Error", "Invalid User ID");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete Staff",
        "Are you sure you want to delete the selected staff member?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        deleteStaffFromDatabase(userId);
    }
}

size_t StaffManagement::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    size_t totalSize = size * nmemb;
    s->append((char*)contents, totalSize);
    return totalSize;
}