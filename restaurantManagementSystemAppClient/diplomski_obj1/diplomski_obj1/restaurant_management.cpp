#include "restaurant_management.h"  
#include "main_panel.h"
#include <sqlite3.h>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QDateTime>
#include <QByteArray>
#include <QString>
#include <QCheckBox>  
#include <QLabel>  
#include "Customer_registration.h"

restaurant_management::restaurant_management(QWidget* parent)
    : QMainWindow(parent)
{
    if (checkForExistingSession()) {
        return;
    }

    setWindowTitle("Restaurant Management System - Login");
    setWindowIcon(QPixmap("logo.png"));
    setFixedSize(800, 600);   
 
    QLabel* logoLabel = new QLabel;
    logoLabel->setPixmap(QPixmap("logo.png")
        .scaled(90, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);

    QLabel* usernameLabel = new QLabel("Username:");
    QLabel* passwordLabel = new QLabel("Password: ");

    usernameLineEdit = new QLineEdit();
    passwordLineEdit = new QLineEdit();
    passwordLineEdit->setEchoMode(QLineEdit::Password); 
    usernameLineEdit->setPlaceholderText("Enter your username");
    passwordLineEdit->setPlaceholderText("Enter your password");

    rememberMeCheckBox = new QCheckBox("Remember Me");
    rememberMeCheckBox->setStyleSheet(
        "QCheckBox { color: #333333; font-size: 16px; background-color: #F0F0F0; padding: 5px; border: 1px solid gray; border-radius: 5px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        "QCheckBox::indicator:unchecked { border: 1px solid #555555; }"
    );

    loginButton = new QPushButton("Login");
    exitButton = new QPushButton("Exit");

    customerRegistrationButton = new QPushButton("Customer Registration");

    connect(loginButton, &QPushButton::clicked, this, &restaurant_management::onLoginClicked);
    connect(exitButton, &QPushButton::clicked, this, &restaurant_management::onExitClicked);
    connect(customerRegistrationButton, &QPushButton::clicked, this, &restaurant_management::onCustomerRegistrationClicked);

    QHBoxLayout* usernameLayout = new QHBoxLayout();
    usernameLayout->addWidget(usernameLabel);
    usernameLayout->addWidget(usernameLineEdit);

    QHBoxLayout* passwordLayout = new QHBoxLayout();
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordLineEdit);

    QVBoxLayout* formLayout = new QVBoxLayout();
    formLayout->setSpacing(15);
    formLayout->setContentsMargins(20, 20, 20, 20); 
    formLayout->addWidget(logoLabel, 0, Qt::AlignCenter); 
    formLayout->addLayout(usernameLayout); 
    formLayout->addLayout(passwordLayout); 
    formLayout->addWidget(rememberMeCheckBox, 0, Qt::AlignLeft); 
    formLayout->addSpacing(20); 

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loginButton, 0, Qt::AlignRight);
    buttonLayout->addWidget(exitButton, 0, Qt::AlignLeft);
    formLayout->addLayout(buttonLayout);

    QWidget* formWidget = new QWidget();
    formWidget->setLayout(formLayout);
    formWidget->setFixedWidth(400); 
    formWidget->setStyleSheet(
        "background-color: rgba(255, 255, 255, 0.9);"
        "border-radius: 10px;"
        "padding: 25px;"
        "box-shadow: 0px 8px 20px rgba(0, 0, 0, 0.3);"
    );

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addStretch();
    mainLayout->addWidget(formWidget, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch(); 
    bottomLayout->addWidget(customerRegistrationButton, 0, Qt::AlignRight);
    bottomLayout->setContentsMargins(20, 10, 20, 20);

    QVBoxLayout* finalLayout = new QVBoxLayout();
    finalLayout->addLayout(mainLayout);
    finalLayout->addLayout(bottomLayout);

    QWidget* centralWidget = new QWidget(this);
    centralWidget->setLayout(finalLayout);
    centralWidget->setStyleSheet(
        "background: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, "
        "stop:0 rgba(52, 152, 219, 1), stop:1 rgba(41, 128, 185, 1));"
    );
    setCentralWidget(centralWidget);
 
    usernameLabel->setStyleSheet("color: #333333; font-size: 16px;");
    passwordLabel->setStyleSheet("color: #333333; font-size: 16px;");
    usernameLineEdit->setStyleSheet(
        "padding: 12px; font-size: 14px; background-color: #ECF0F1; color: #333333;"
        "border: 1px solid gray; border-radius: 6px;"
    );
    passwordLineEdit->setStyleSheet(
        "padding: 12px; font-size: 14px; background-color: #ECF0F1; color: #333333;"
        "border: 1px solid gray; border-radius: 6px;"
    );
    loginButton->setStyleSheet(
        "QPushButton { padding: 12px; font-size: 14px; background-color: #2ECC71; color: white;"
        "border-radius: 6px; }"
        "QPushButton:hover { background-color: #27AE60; }"
    );
    exitButton->setStyleSheet(
        "QPushButton { padding: 12px; font-size: 14px; background-color: #E74C3C; color: white;"
        "border-radius: 6px; }"
        "QPushButton:hover { background-color: #C0392B; }"
    );
    customerRegistrationButton->setStyleSheet(
        "QPushButton { padding: 10px; font-size: 14px; background-color: #3498db; color: white; border: none;"
        "border-radius: 6px; }"
        "QPushButton:hover { background-color: #2980b9; }"
    );
}


restaurant_management::~restaurant_management() {}

void restaurant_management::onLoginClicked() {
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();

    QByteArray hashedPassword = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();

    QJsonObject json;
    json["username"] = username;
    json["password_hash"] = QString(hashedPassword);

    QJsonDocument doc(json);
    std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();
    
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/login";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);   

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(response_string).toUtf8());
            QJsonObject jsonObject = jsonResponse.object();

            if (jsonObject["status"] == "success") {
                QString role = jsonObject["role"].toString();
                int userId = jsonObject["user_id"].toInt();

                if (rememberMeCheckBox->isChecked()) {
                    storeSession(userId);
                }

                handleLoginSuccess(username, role);
            }
            else {
                QString errorMsg = jsonObject["message"].toString();
                QMessageBox::warning(this, "Login Failed", errorMsg);
            }
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to send login request. Network issue.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }
}

void restaurant_management::logAuditAction(const QString& username, const QString& action) {
    QJsonObject json;
    json["username"] = username;
    json["action"] = action;

    QJsonDocument doc(json);
    std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/log_audit";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            qWarning() << "Failed to log audit action. Network issue.";
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        qWarning() << "Failed to initialize CURL for audit logging.";
    }
}

void restaurant_management::storeSession(int userId) {
    QString sessionToken = QString::fromLatin1(QCryptographicHash::hash(QDateTime::currentDateTime().toString().toUtf8(), QCryptographicHash::Sha256).toHex());
    QString expirationTime = QDateTime::currentDateTime().addDays(30).toString(Qt::ISODate);

    QJsonObject json;
    json["user_id"] = userId;
    json["session_token"] = sessionToken;
    json["expires_at"] = expirationTime;

    QJsonDocument doc(json);
    std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/store_session";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QMessageBox::information(this, "Session", "Session stored successfully.");
        }
        else {
            qWarning() << "Failed to store session. Network issue.";
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        qWarning() << "Failed to initialize CURL for session storage.";
    }
}



bool restaurant_management::checkForExistingSession() {
    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/check_session";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string); 

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(QString::fromStdString(response_string).toUtf8());
            QJsonObject jsonObject = jsonResponse.object();

            if (jsonObject["status"] == "success") {
                QJsonObject sessionData = jsonObject["session"].toObject();
                int userId = sessionData["user_id"].toInt();
                QString username = sessionData["username"].toString();
                QString role = sessionData["role"].toString();

                handleLoginSuccess(username, role);
                curl_easy_cleanup(curl);
                curl_slist_free_all(headers);

                return true;
            }
            else {
                QString errorMsg = jsonObject["message"].toString();
                qDebug() << "Session error:" << errorMsg;
            }
        }
        else {
            qDebug() << "CURL Error:" << curl_easy_strerror(res);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }

    return false;
}

size_t restaurant_management::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void restaurant_management::handleLoginSuccess(QString username, QString role) {
    QString roleId = "";
    if (role == "administrator") roleId += "1";
    else if (role == "waiter") roleId += "2";
    else if (role == "manager") roleId += "3";
    else if (role == "kitchen_staff") roleId += "4";
    else if (role == "customer") roleId += "5";

    QString tempRole = role;
    tempRole[0] = tempRole[0].toUpper();
    QMessageBox::information(this, "Login Successful", "Welcome to the system, " + tempRole);

    logAuditAction(username, "Successful login as " + tempRole);
    if (role == "administrator" || role == "waiter" || role == "manager" || role == "kitchen_staff") {
        automaticCheckIn(username);
    }

    emit loggedIn(role);

    if (mainPanel != nullptr) {
        delete mainPanel;
        mainPanel = nullptr;
    }

    mainPanel = new MainPanel(username, roleId);
    mainPanel->show();

    this->close();
}

void restaurant_management::automaticCheckIn(const QString& username) {
    int userId = getUserIdByUsername(username);

    if (userId == -1) {
        QMessageBox::critical(this, "Error", "User not found.");
        return;
    }

    QJsonObject json;
    json["user_id"] = userId;

    QJsonDocument doc(json);
    std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/check_in";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            QMessageBox::information(this, "Check-In", "Check-in successful.");
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to check in. Network issue.");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }
}

int restaurant_management::getUserIdByUsername(const QString& username) {
    CURL* curl;
    CURLcode res;
    int userId = -1;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_user_id";

        QJsonObject json;
        json["username"] = username;

        QJsonDocument doc(json);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

        std::string response_string;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, MainPanel::WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            QString errorMsg = QString("cURL request failed: %1").arg(curl_easy_strerror(res));
            QMessageBox::critical(this, "Error", errorMsg);
        }
        else {
            QJsonDocument responseDoc = QJsonDocument::fromJson(QString::fromStdString(response_string).toUtf8());
            if (!responseDoc.isNull() && responseDoc.isObject()) {
                QJsonObject responseObject = responseDoc.object();
                if (responseObject.contains("user_id")) {
                    userId = responseObject["user_id"].toInt();
                }
                else if (responseObject.contains("error")) {
                    QMessageBox::critical(this, "Error", responseObject["error"].toString());
                }
            }
            else {
                QMessageBox::critical(this, "Error", "Invalid JSON response.");
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize cURL.");
    }

    return userId;
}

void restaurant_management::onCustomerRegistrationClicked() {
    this->close();

    CustomerRegistration* registrationForm = new CustomerRegistration();
    registrationForm->setAttribute(Qt::WA_DeleteOnClose); 
    registrationForm->show();
}

void restaurant_management::onExitClicked()
{
    if (mainPanel != nullptr) {
        delete mainPanel;
        mainPanel = nullptr;
    }
    //tableManagement->releaseAllTables();

    close();
}

void restaurant_management::closeEvent(QCloseEvent* event)
{
    //tableManagement->releaseAllTables();
}