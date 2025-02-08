#include "Profile.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFont>
#include <QHBoxLayout>
#include <QSpacerItem>

Profile::Profile(int userId, QWidget* parent) : QWidget(parent), userId(userId)
{
    setWindowTitle("User Profile");

    setStyleSheet("background-color: #ffffff; color: #2c3e50;");
    QFont labelFont("Arial", 13, QFont::Bold); 
    QFont dataFont("Arial", 12);  
    QFont buttonFont("Arial", 12, QFont::Bold);  

    QLabel* titleLabel = new QLabel("User Profile");
    titleLabel->setFont(QFont("Arial", 18, QFont::Bold));  
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("padding: 15px; color: #2980b9; background-color: #ecf0f1; border-radius: 10px;");

    labelFullName = new QLabel("Full Name:");
    labelFullName->setFont(labelFont);
    labelFullName->setStyleSheet("color: #34495e;");

    editFullNameField = new QLineEdit();
    editFullNameField->setFont(dataFont);
    editFullNameField->setVisible(false);  
    editFullNameField->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 5px;");

    editNameButton = new QPushButton("Edit");
    editNameButton->setFont(buttonFont);
    editNameButton->setStyleSheet("padding: 8px 20px; background-color: #3498db; color: white; border: none; border-radius: 5px;");
    connect(editNameButton, &QPushButton::clicked, this, &Profile::onEditName);

    labelEmail = new QLabel("Email:");
    labelEmail->setFont(labelFont);
    labelEmail->setStyleSheet("color: #34495e;");

    editEmailField = new QLineEdit();
    editEmailField->setFont(dataFont);
    editEmailField->setVisible(false);
    editEmailField->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 5px;");

    editEmailButton = new QPushButton("Edit");
    editEmailButton->setFont(buttonFont);
    editEmailButton->setStyleSheet("padding: 8px 20px; background-color: #3498db; color: white; border: none; border-radius: 5px;");
    connect(editEmailButton, &QPushButton::clicked, this, &Profile::onEditEmail);

    labelPhone = new QLabel("Phone:");
    labelPhone->setFont(labelFont);
    labelPhone->setStyleSheet("color: #34495e;");

    editPhoneField = new QLineEdit();
    editPhoneField->setFont(dataFont);
    editPhoneField->setVisible(false);
    editPhoneField->setStyleSheet("padding: 5px; border: 1px solid #bdc3c7; border-radius: 5px;");

    editPhoneButton = new QPushButton("Edit");
    editPhoneButton->setFont(buttonFont);
    editPhoneButton->setStyleSheet("padding: 8px 20px; background-color: #3498db; color: white; border: none; border-radius: 5px;");
    connect(editPhoneButton, &QPushButton::clicked, this, &Profile::onEditPhone);

    labelRole = new QLabel("Role:");
    labelRole->setFont(labelFont);
    labelRole->setStyleSheet("color: #34495e;");

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    mainLayout->addWidget(titleLabel);

    auto createEditableRow = [&](QLabel* label, QLineEdit* editField, QPushButton* editButton) {
        QHBoxLayout* hLayout = new QHBoxLayout();
        hLayout->addWidget(label);
        hLayout->addWidget(editField);
        hLayout->addWidget(editButton);
        return hLayout;
    };

    mainLayout->addLayout(createEditableRow(labelFullName, editFullNameField, editNameButton));
    mainLayout->addLayout(createEditableRow(labelEmail, editEmailField, editEmailButton));
    mainLayout->addLayout(createEditableRow(labelPhone, editPhoneField, editPhoneButton));
    mainLayout->addWidget(labelRole);

    QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    mainLayout->addSpacerItem(spacer);

    setLayout(mainLayout);

    fetchProfileData(userId);
}

Profile::~Profile()
{
}

size_t Profile::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void Profile::fetchProfileData(int userId)
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_user_info/" + std::to_string(userId);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            QMessageBox::critical(this, "Error", "Failed to fetch data: " + QString(curl_easy_strerror(res)));
        else
            parseProfileData(QString::fromStdString(readBuffer));

        curl_easy_cleanup(curl);
    }
}

void Profile::parseProfileData(const QString& jsonData)
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData.toUtf8());

    if (jsonDoc.isObject()) {
        QJsonObject jsonObj = jsonDoc.object();

        QString fullName = jsonObj["full_name"].toString();
        QString email = jsonObj["email"].toString();
        QString role = jsonObj["role"].toString();
        QString phone = jsonObj["phone"].toString();

        labelFullName->setText("Full Name: " + fullName);
        labelEmail->setText("Email: " + email);
        labelRole->setText("Role: " + role);
        labelPhone->setText("Phone: " + phone);
    }
    else {
        QMessageBox::warning(this, "Error", "Invalid JSON response");
    }
}

void Profile::onEditName()
{
    toggleEditMode(labelFullName, editFullNameField, editNameButton);
}

void Profile::onEditEmail()
{
    toggleEditMode(labelEmail, editEmailField, editEmailButton);
}

void Profile::onEditPhone()
{
    toggleEditMode(labelPhone, editPhoneField, editPhoneButton);
}

void Profile::toggleEditMode(QLabel* label, QLineEdit* editField, QPushButton* button)
{
    if (editField->isVisible()) {
        sendUpdateRequest(label->text().split(":")[0], editField->text());
        label->setText(label->text().split(":")[0] + ": " + editField->text());
        editField->setVisible(false);
        label->setVisible(true);
        button->setText("Edit");
    }
    else {
        editField->setText(label->text().split(":")[1].trimmed());
        editField->setVisible(true);
        label->setVisible(false);
        button->setText("Change");
    }
}

void Profile::sendUpdateRequest(const QString& field, const QString& newValue) {
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/update_profile";

        QString jsonField;
        if (field == "Full Name") {
            jsonField = "full_name";
        }
        else if (field == "Email") {
            jsonField = "email";
        }
        else if (field == "Phone") {
            jsonField = "phone";
        }
        else {
            QMessageBox::warning(this, "Error", "Invalid field");
            return;
        }

        QJsonObject json;
        json["user_id"] = userId;
        json[jsonField] = newValue;

        QJsonDocument doc(json);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QMessageBox::information(this, "Success", field + " updated successfully.");
        }
        else {
            QMessageBox::warning(this, "Error", "Failed to update " + field + ": " + QString(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
}