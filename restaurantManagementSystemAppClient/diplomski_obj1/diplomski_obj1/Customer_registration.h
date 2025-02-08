#ifndef CUSTOMER_REGISTRATION_H
#define CUSTOMER_REGISTRATION_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QCheckBox>
#include <QFormLayout>
#include "sqlite3.h"

class CustomerRegistration : public QWidget
{
    Q_OBJECT

public:
    explicit CustomerRegistration(QWidget* parent = nullptr);

private slots:
    void onRegisterClicked();  
    bool registerCustomer(const QString& username, const QString& passwordHash, const QString& fullName, const QString& email, const QString& phone);
    
private:
    QLineEdit* usernameLineEdit;
    QLineEdit* passwordLineEdit;
    QLineEdit* fullNameLineEdit;
    QLineEdit* emailLineEdit;
    QLineEdit* phoneLineEdit;

    QPushButton* registerButton;
    QPushButton* cancelButton;

    //bool saveCustomerToDatabase(const QString& username, const QString& passwordHash, const QString& fullName, const QString& email, const QString& phone);
    QString hashPassword(const QString& password); 
};

#endif


