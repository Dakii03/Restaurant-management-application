#ifndef MAIN_PANEL_H
#define MAIN_PANEL_H

#include <QtWidgets/QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QStackedWidget>
#include "table_management.h"
#include "Menu.h"
#include "Order.h"
#include "Reservation.h"
#include "Billing.h"
#include "Staff_management.h" 
#include <QCryptographicHash>
#include "sqlite3.h"
#include "CustomerMenu.h"
#include "Cart.h"
#include "Profile.h"
#include "KitchenOrderManager.h"
#include <curl/curl.h>

class MainPanel : public QMainWindow{
    Q_OBJECT

public:
    explicit MainPanel(const QString& username = "", const QString& roleId = "", QWidget* parent = nullptr);
    ~MainPanel();
    static QString getUserRoleFromDatabase(const QString& username);
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void showTableManagement();
    void showMenuManagement();
    void showOrderManagement();
    void showReservationSystem();
    void showBillingSystem();
    void showStaffManagement();
    void showCustomerMenu();
    void showCustomerCart();
    void showProfile();
    void showOnlineCustomerOrders();

    void logout();
    int getUserIdByUsername(const QString& username);
    QString hashPassword(const QString& password);

    QList<QString> getPermissionsFromDatabase(const QString& role_id);
    void registerNewStaffMember();
signals:
    void loggedOut();
private:
    QStackedWidget* centralWidgetStack;
    
    TableManagement* tableManagementWidget;
    MenuManagement* menuManagementWidget;
    Order* orderManagementWidget;
    Reservation* reservationManagementWidget;
    Billing* billingWidget;
    StaffManagement* staffManagementWidget;
    CustomerMenu* customerMenuWidget;
    Cart* customerCartWidget;
    Profile* profileWidget;
    KitchenOrderManager* kitchenOrderManagerWidget;

    QAction* tableAction;
    QAction* menuAction;
    QAction* orderAction;
    QAction* reservationAction;
    QAction* billingAction;
    QAction* staffAction;
    QAction* logoutAction;
    QAction* customerMenuAction;
    QAction* customerCartAction;
    QAction* profileAction;
    QAction* kitchenAction;

    QString currentUsername;
    QString tempUsername;
    QString currentRoleId;

};

#endif 