#pragma once

#include <QtWidgets/QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QCheckBox>
#include "main_panel.h"
#include "table_management.h"
#include <curl/curl.h>

class restaurant_management : public QMainWindow
{
    Q_OBJECT

public:
    restaurant_management(QWidget* parent = nullptr);
    ~restaurant_management();
    bool checkForExistingSession();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onLoginClicked();    
    void onExitClicked();    
    void logAuditAction(const QString& username, const QString& action);
    void storeSession(int userId);
    void handleLoginSuccess(QString username, QString role);
    void automaticCheckIn(const QString& username);
    int getUserIdByUsername(const QString& username);
    void onCustomerRegistrationClicked();
signals:
    void loggedIn(const QString& role);

private:
    QLineEdit* usernameLineEdit;  
    QLineEdit* passwordLineEdit;  
    QPushButton* loginButton;     
    QPushButton* exitButton;   
    QPushButton* customerRegistrationButton;
    MainPanel* mainPanel = nullptr;
    TableManagement* tableManagement;
    QCheckBox* rememberMeCheckBox;
    QString tempPassword;
};
