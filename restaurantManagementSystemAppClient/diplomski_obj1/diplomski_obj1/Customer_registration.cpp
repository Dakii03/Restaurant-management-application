#include "Customer_registration.h"
#include "restaurant_management.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QMessageBox>
#include <curl/curl.h>

CustomerRegistration::CustomerRegistration(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("Customer Registration");
    setFixedSize(800, 600);  

    QLabel* titleLabel = new QLabel("Register New Customer");
    QLabel* usernameLabel = new QLabel("Username:");
    QLabel* passwordLabel = new QLabel("Password:");
    QLabel* fullNameLabel = new QLabel("Full Name:");
    QLabel* emailLabel = new QLabel("Email:");
    QLabel* phoneLabel = new QLabel("Phone:");

    usernameLineEdit = new QLineEdit();
    passwordLineEdit = new QLineEdit();
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    fullNameLineEdit = new QLineEdit();
    emailLineEdit = new QLineEdit();
    phoneLineEdit = new QLineEdit();

    registerButton = new QPushButton("Register");
    cancelButton = new QPushButton("Cancel");

    titleLabel->setStyleSheet("font-size: 24px; color: #FFFFFF; padding-bottom: 20px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    usernameLabel->setStyleSheet("color: #FFFFFF; font-size: 16px;");
    passwordLabel->setStyleSheet("color: #FFFFFF; font-size: 16px;");
    fullNameLabel->setStyleSheet("color: #FFFFFF; font-size: 16px;");
    emailLabel->setStyleSheet("color: #FFFFFF; font-size: 16px;");
    phoneLabel->setStyleSheet("color: #FFFFFF; font-size: 16px;");

    usernameLineEdit->setStyleSheet("padding: 10px; background-color: #3A3A3A; color: white; border: 1px solid #555555; border-radius: 5px;");
    passwordLineEdit->setStyleSheet("padding: 10px; background-color: #3A3A3A; color: white; border: 1px solid #555555; border-radius: 5px;");
    fullNameLineEdit->setStyleSheet("padding: 10px; background-color: #3A3A3A; color: white; border: 1px solid #555555; border-radius: 5px;");
    emailLineEdit->setStyleSheet("padding: 10px; background-color: #3A3A3A; color: white; border: 1px solid #555555; border-radius: 5px;");
    phoneLineEdit->setStyleSheet("padding: 10px; background-color: #3A3A3A; color: white; border: 1px solid #555555; border-radius: 5px;");

    registerButton->setStyleSheet("padding: 10px; font-size: 16px; background-color: #4CAF50; color: white; border: none; border-radius: 5px;");
    cancelButton->setStyleSheet("padding: 10px; font-size: 16px; background-color: #f44336; color: white; border: none; border-radius: 5px;");

    QFormLayout* formLayout = new QFormLayout();
    formLayout->setSpacing(15);
    formLayout->addRow(usernameLabel, usernameLineEdit);
    formLayout->addRow(passwordLabel, passwordLineEdit);
    formLayout->addRow(fullNameLabel, fullNameLineEdit);
    formLayout->addRow(emailLabel, emailLineEdit);
    formLayout->addRow(phoneLabel, phoneLineEdit);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(registerButton);
    buttonLayout->addWidget(cancelButton);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(30);
    mainLayout->addLayout(buttonLayout, Qt::AlignCenter);

    setLayout(mainLayout);

    setStyleSheet("background-color: #2980b9");

    // Connect signals to slots
    connect(registerButton, &QPushButton::clicked, this, &CustomerRegistration::onRegisterClicked);
    connect(cancelButton, &QPushButton::clicked, this, &CustomerRegistration::close);
}

void CustomerRegistration::onRegisterClicked()
{
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();
    QString fullName = fullNameLineEdit->text();
    QString email = emailLineEdit->text();
    QString phone = phoneLineEdit->text();

    if (username.isEmpty() || password.isEmpty() || fullName.isEmpty() || email.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all required fields.");
        return;
    }

    QString passwordHash = hashPassword(password);

    if (registerCustomer(username, passwordHash, fullName, email, phone)) {
        QMessageBox::information(this, "Success", "Customer registered successfully!");
        this->close();
        restaurant_management* loginScreen = new restaurant_management();
        loginScreen->setAttribute(Qt::WA_DeleteOnClose);
        loginScreen->show();
    }
    else {
        QMessageBox::warning(this, "Error", "Failed to register customer. Please try again.");
    }
}

QString CustomerRegistration::hashPassword(const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool CustomerRegistration::registerCustomer(const QString& username, const QString& passwordHash, const QString& fullName, const QString& email, const QString& phone)
{
    CURL* curl;
    CURLcode res;
    bool success = false;

    // Initialize cURL
    curl = curl_easy_init();
    if (curl) {
        // JSON payload
        std::string jsonData = "{\"username\":\"" + username.toStdString() + "\", \"password_hash\":\"" + passwordHash.toStdString() + "\", \"full_name\":\"" + fullName.toStdString() + "\", \"email\":\"" + email.toStdString() + "\", \"phone\":\"" + phone.toStdString() + "\"}";

        // Set cURL options
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/register_customer");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());

        // Set headers
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            success = true;
        }
        else {
            qDebug() << "cURL request failed: " << curl_easy_strerror(res);
        }

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    return success;
}