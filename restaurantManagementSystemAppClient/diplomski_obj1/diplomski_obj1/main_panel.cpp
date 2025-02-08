#include "main_panel.h"
#include "restaurant_management.h"
#include "SetUsernamePasswordDialog.h"
#include <QLabel>
#include <QMessageBox>
#include "sqlite3.h"
#include <QJsonObject>
#include <QJsonDocument>

MainPanel::MainPanel(const QString& username, const QString& roleId, QWidget* parent)
    : QMainWindow(parent), tableManagementWidget(nullptr)
{
    setWindowTitle("Restaurant Management System");
    setWindowIcon(QPixmap("logo.png"));
    setFixedSize(1000, 600);

    QMenuBar* menuBar = new QMenuBar(this);

    QMenu* tableMenu = new QMenu("Tables", this);
    QMenu* menuManagement = new QMenu("Menu", this);
    QMenu* orderMenu = new QMenu("Orders", this);
    QMenu* reservationMenu = new QMenu("Reservations", this);
    QMenu* billingMenu = new QMenu("Billing", this);
    QMenu* staffMenu = new QMenu("Staff");
    QMenu* accountMenu = new QMenu("Account", this);
    QMenu* customerMenu = new QMenu("Menu", this);
    QMenu* customerCartMenu = new QMenu("Cart", this);
    QMenu* kitchenOrderMenu = new QMenu("Online orders", this);

    bool isNumeric;
    username.toInt(&isNumeric);

    currentUsername = username;
    tempUsername = username;
    currentRoleId = roleId;

    if (isNumeric) {
        registerNewStaffMember();
    }

    //QMessageBox::information(this, "debug", currentUsername + " " + currentRoleId);

    tableAction = tableMenu->addAction("Manage Tables");
    menuAction = menuManagement->addAction("Manage Menu");
    orderAction = orderMenu->addAction("Manage Orders");
    reservationAction = reservationMenu->addAction("Manage Reservations");
    billingAction = billingMenu->addAction("Billing");
    staffAction = staffMenu->addAction("Staff management");
    customerMenuAction = customerMenu->addAction("Show menu");
    profileAction = accountMenu->addAction("Profile");
    logoutAction = accountMenu->addAction("Logout");
    customerCartAction = customerCartMenu->addAction("Show cart");
    kitchenAction = kitchenOrderMenu->addAction("Manage Online Orders");

    tableAction->setEnabled(false);
    menuAction->setEnabled(false);
    orderAction->setEnabled(false);
    reservationAction->setEnabled(false);
    billingAction->setEnabled(false);
    staffAction->setEnabled(false);
    customerMenuAction->setEnabled(false);
    customerCartAction->setEnabled(false);
    kitchenAction->setEnabled(false);

    QList<QString> permissions = getPermissionsFromDatabase(roleId);
    
    if (permissions.contains("Manage Tables")) {
        tableAction->setEnabled(true);
        menuBar->addMenu(tableMenu);
    }
    if (permissions.contains("Manage Menu")) {
        menuAction->setEnabled(true);
        menuBar->addMenu(menuManagement);
    }
    if (permissions.contains("Manage Orders")) {
        orderAction->setEnabled(true);
        menuBar->addMenu(orderMenu);
    }
    if (permissions.contains("Manage Reservations")) {
        reservationAction->setEnabled(true);
        menuBar->addMenu(reservationMenu);
    }
    if (permissions.contains("Billing")) {
        billingAction->setEnabled(true);
        menuBar->addMenu(billingMenu);
    }
    if (permissions.contains("Manage Staff")) {
        staffAction->setEnabled(true);  
        menuBar->addMenu(staffMenu);
    }
    if (permissions.contains("Customer Menu")) {
        customerMenuAction->setEnabled(true);
        menuBar->addMenu(customerMenu);
    }
    if (permissions.contains("Customer Billing")) {
        customerCartAction->setEnabled(true);
        menuBar->addMenu(customerCartMenu);
    }
    if (permissions.contains("Manage Customer Order")) {
        kitchenAction->setEnabled(true);
        menuBar->addMenu(kitchenOrderMenu);
    }

    menuBar->addMenu(accountMenu);  

    setMenuBar(menuBar);  

    centralWidgetStack = new QStackedWidget(this);

    int userId = getUserIdByUsername(currentUsername);

    customerMenuWidget = new CustomerMenu(userId, this);
    customerCartWidget = new Cart(userId, this);
    profileWidget = new Profile(userId, this);
    tableManagementWidget = new TableManagement(this); 
    menuManagementWidget = new MenuManagement(currentUsername);
    orderManagementWidget = new Order(this);
    reservationManagementWidget = new Reservation(this);
    billingWidget = new Billing(this);
    staffManagementWidget = new StaffManagement(this);
    kitchenOrderManagerWidget = new KitchenOrderManager(4, currentUsername, this);

    // Add widgets to the stacked layout
    centralWidgetStack->addWidget(tableManagementWidget);             // Index 0
    centralWidgetStack->addWidget(menuManagementWidget);              // Index 1
    centralWidgetStack->addWidget(orderManagementWidget);             // Index 2
    centralWidgetStack->addWidget(reservationManagementWidget);       // Index 3
    centralWidgetStack->addWidget(billingWidget);                     // Index 4
    centralWidgetStack->addWidget(staffManagementWidget);             // Index 5
    centralWidgetStack->addWidget(customerMenuWidget);                // Index 6
    centralWidgetStack->addWidget(customerCartWidget);                // Index 7
    centralWidgetStack->addWidget(profileWidget);                     // Index 8
    centralWidgetStack->addWidget(kitchenOrderManagerWidget);         // Index 9

    if (getUserRoleFromDatabase(currentUsername) == "manager") {
        centralWidgetStack->setCurrentIndex(3);
    }
    else if (!permissions.contains("Customer Menu") && !permissions.contains("Manage Customer Order")) {
        centralWidgetStack->setCurrentIndex(0); // Inicijalno prikazi table management
    }
    else if (permissions.contains("Manage Customer Order")) {
        centralWidgetStack->setCurrentIndex(9); // Kitchen order management
    }
    else {
        centralWidgetStack->setCurrentIndex(6); // Customer menu
    }

    setCentralWidget(centralWidgetStack);

    connect(tableAction, &QAction::triggered, this, &MainPanel::showTableManagement);
    connect(menuAction, &QAction::triggered, this, &MainPanel::showMenuManagement);
    connect(orderAction, &QAction::triggered, this, &MainPanel::showOrderManagement);
    connect(reservationAction, &QAction::triggered, this, &MainPanel::showReservationSystem);
    connect(billingAction, &QAction::triggered, this, &MainPanel::showBillingSystem);
    connect(staffAction, &QAction::triggered, this, &MainPanel::showStaffManagement);
    connect(customerMenuAction, &QAction::triggered, this, &MainPanel::showCustomerMenu);
    connect(customerCartAction, &QAction::triggered, this, &MainPanel::showCustomerCart);
    connect(profileAction, &QAction::triggered, this, &MainPanel::showProfile);
    connect(kitchenAction, &QAction::triggered, this, &MainPanel::showOnlineCustomerOrders);
    connect(logoutAction, &QAction::triggered, this, &MainPanel::logout);

    connect(tableManagementWidget, &TableManagement::tableStatusChanged, orderManagementWidget, &Order::refreshTables);
    connect(orderManagementWidget, &Order::orderPlaced, billingWidget, &Billing::loadOccupiedTables);
    connect(orderManagementWidget, &Order::orderPlaced, menuManagementWidget, &MenuManagement::populateMenuFromDatabase);
    connect(reservationManagementWidget, &Reservation::reservationsLoaded, tableManagementWidget, &TableManagement::refreshTables);
    connect(billingWidget, &Billing::paymentFinalized, tableManagementWidget, &TableManagement::handlePaymentFinalization);
    connect(billingWidget, &Billing::everythingPaid, tableManagementWidget, &TableManagement::releaseAllTables);
    connect(billingWidget, &Billing::paymentFinalized, orderManagementWidget, &Order::refreshTables);
    connect(customerMenuWidget, &CustomerMenu::addedToCart, customerCartWidget, &Cart::loadCartItems);
    if (getUserRoleFromDatabase(currentUsername) == "customer") {
        connect(this, &MainPanel::loggedOut, customerCartWidget, &Cart::handleDeleteCart);
    }

    reservationManagementWidget->loadReservationsFromDatabase();
}


MainPanel::~MainPanel() {}

void MainPanel::closeEvent(QCloseEvent* event)
{
    if (this != nullptr) {
        delete this;
    }
}

void MainPanel::showTableManagement()
{
    centralWidgetStack->setCurrentIndex(0);  // Show Table Management
}

void MainPanel::showMenuManagement()
{
    centralWidgetStack->setCurrentIndex(1);  // Show Menu Management
}

void MainPanel::showOrderManagement()
{
    centralWidgetStack->setCurrentIndex(2);  // Show Order Management
}

void MainPanel::showReservationSystem()
{
    centralWidgetStack->setCurrentIndex(3);  // Show Reservation System
}

void MainPanel::showBillingSystem()
{
    centralWidgetStack->setCurrentIndex(4);  // Show Billing System
}

void MainPanel::showStaffManagement()
{
    centralWidgetStack->setCurrentIndex(5);
}

void MainPanel::showCustomerMenu()
{
    centralWidgetStack->setCurrentIndex(6); // Show customer menu 
}

void MainPanel::showCustomerCart()
{
    centralWidgetStack->setCurrentIndex(7); // Show customer cart
}

void MainPanel::showProfile()
{
    centralWidgetStack->setCurrentIndex(8); // Show profile
}

void MainPanel::showOnlineCustomerOrders()
{
    centralWidgetStack->setCurrentIndex(9); // Show online customer orders
}


void MainPanel::logout() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Logout", "Are you sure you want to log out?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        int userId = getUserIdByUsername(currentUsername);

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
            std::string url = "http://127.0.0.1:5000/logout";
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                QMessageBox::information(this, "Success", "Logout successful.");
                emit loggedOut();

                this->close();

                restaurant_management* loginScreen = new restaurant_management();
                loginScreen->show();
            }
            else {
                QMessageBox::critical(this, "Error", "Failed to logout. Network issue.");
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
        }
    }
}

int MainPanel::getUserIdByUsername(const QString& username) {
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

QString MainPanel::hashPassword(const QString& password)
{
    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

QString MainPanel::getUserRoleFromDatabase(const QString& username)
{
    CURL* curl = curl_easy_init();
    QString roleName;

    if (curl) {
        std::string url = "http://127.0.0.1:5000/get_user_role";

        QJsonObject json;
        json["username"] = username;
        QJsonDocument doc(json);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            QMessageBox::critical(nullptr, "Error", "Failed to connect to the server.");
        }
        else {
            QJsonDocument responseDoc = QJsonDocument::fromJson(QString::fromStdString(response_string).toUtf8());
            if (!responseDoc.isNull() && responseDoc.isObject()) {
                QJsonObject responseObject = responseDoc.object();
                if (responseObject.contains("role_name")) {
                    roleName = responseObject["role_name"].toString();
                }
                else if (responseObject.contains("error")) {
                    QMessageBox::critical(nullptr, "Error", responseObject["error"].toString());
                }
            }
            else {
                QMessageBox::critical(nullptr, "Error", "Failed to parse response.");
            }
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(nullptr, "Error", "Failed to initialize cURL.");
    }

    return roleName;
}

size_t MainPanel::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp)
{
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}


QList<QString> MainPanel::getPermissionsFromDatabase(const QString& role_id)
{
    QList<QString> permissions;

    CURL* curl = curl_easy_init();
    if (curl) {
        QJsonObject json;
        json["role_id"] = role_id;

        QJsonDocument doc(json);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

        std::string url = "http://127.0.0.1:5000/get_permissions";
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
            if (!responseDoc.isNull() && responseDoc["status"].toString() == "success") {
                QJsonArray permissionsArray = responseDoc["permissions"].toArray();
                for (const QJsonValue& value : permissionsArray) {
                    permissions.append(value.toString());
                }
            }
            else {
                QString errorMessage = responseDoc["message"].toString();
                QMessageBox::warning(this, "Error", QString("Server Error: %1").arg(errorMessage));
            }
        }
        else {
            QMessageBox::critical(this, "Error", QString("Failed to get permissions: %1").arg(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to initialize CURL.");
    }

    return permissions;
}

void MainPanel::registerNewStaffMember()
{
    SetUsernamePasswordDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString newUsername = dialog.getUsername();
        QString newPassword = dialog.getPassword();
        QString fullName = dialog.getFullName();
        QString email = dialog.getEmail();
        QString phone = dialog.getPhone();

        QString hashedPassword = hashPassword(newPassword);

        QJsonObject jsonData;
        jsonData["new_username"] = newUsername;
        jsonData["hashed_password"] = hashedPassword;
        jsonData["full_name"] = fullName;
        jsonData["email"] = email;
        jsonData["phone"] = phone;
        jsonData["current_username"] = tempUsername;
        
        QJsonDocument doc(jsonData);
        std::string payload = doc.toJson(QJsonDocument::Compact).toStdString();

        CURL* curl = curl_easy_init();
        if (curl) {
            std::string url = "http://127.0.0.1:5000/update_staff_member";
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

                if (responseObject["status"].toString() == "success") {
                    currentUsername = newUsername;
                    QMessageBox::information(this, "Success", "Staff data updated successfully.");
                }
                else {
                    QString errorMessage = responseObject["message"].toString();
                    QMessageBox::critical(this, "Error", QString("Failed to update staff data: %1").arg(errorMessage));
                }
            }
            else {
                QString errorMsg = QString("Failed to update staff data: %1").arg(curl_easy_strerror(res));
                QMessageBox::critical(this, "Error", errorMsg);
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
        else {
            QMessageBox::critical(this, "Error", "Failed to initialize cURL.");
        }
    }
    else {
        this->close();
    }
}