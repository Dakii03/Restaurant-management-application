#include "SetUsernamePasswordDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QRegularExpressionValidator>

SetUsernamePasswordDialog::SetUsernamePasswordDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Restaurant Management System - Staff register");
    setWindowIcon(QPixmap("C:\\Users\\Lenovo\\source\\repos\\diplomski_obj1\\diplomski_obj1\\logo.png"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* fullNameLabel = new QLabel("Full Name:", this);
    fullNameEdit = new QLineEdit(this);
    layout->addWidget(fullNameLabel);
    layout->addWidget(fullNameEdit);

    QLabel* usernameLabel = new QLabel("Username:", this);
    usernameEdit = new QLineEdit(this);
    layout->addWidget(usernameLabel);
    layout->addWidget(usernameEdit);

    QLabel* passwordLabel = new QLabel("Password:", this);
    passwordEdit = new QLineEdit(this);
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordEdit);

    QLabel* emailLabel = new QLabel("Email:", this);
    emailEdit = new QLineEdit(this);
    layout->addWidget(emailLabel);
    layout->addWidget(emailEdit);

    QLabel* phoneLabel = new QLabel("Phone Number:", this);
    phoneEdit = new QLineEdit(this);
    phoneEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("^\\d{10}$"), this)); // Validator for 10-digit phone
    layout->addWidget(phoneLabel);
    layout->addWidget(phoneEdit);

    okButton = new QPushButton("OK", this);
    cancelButton = new QPushButton("Cancel", this);
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);

    connect(okButton, &QPushButton::clicked, this, &SetUsernamePasswordDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &SetUsernamePasswordDialog::reject);

    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);

    setStyleSheet("QDialog {"
        "background-color: #ffffff;"
        "font-size: 14px;"
        "border: 1px solid #cccccc;"
        "border-radius: 8px;"
        "padding: 10px; }"
        "QLabel {"
        "color: #333333; }"
        "QLineEdit {"
        "border: 1px solid #cccccc;"
        "border-radius: 4px;"
        "padding: 5px; }"
        "QPushButton {"
        "background-color: #4CAF50;"
        "color: white;"
        "padding: 8px;"
        "border: none;"
        "border-radius: 5px;"
        "cursor: pointer; }"
        "QPushButton:hover {"
        "background-color: #45a049; }");
}

QString SetUsernamePasswordDialog::getUsername() const
{
    return usernameEdit->text();
}

QString SetUsernamePasswordDialog::getPassword() const
{
    return passwordEdit->text();
}

QString SetUsernamePasswordDialog::getFullName() const
{
    return fullNameEdit->text();
}

QString SetUsernamePasswordDialog::getEmail() const
{
    return emailEdit->text();
}

QString SetUsernamePasswordDialog::getPhone() const
{
    return phoneEdit->text();
}

void SetUsernamePasswordDialog::onOkClicked()
{
    if (!fullNameEdit->text().isEmpty() && !usernameEdit->text().isEmpty() &&
        !passwordEdit->text().isEmpty() && !emailEdit->text().isEmpty() &&
        !phoneEdit->text().isEmpty())
    {
        accept();
    }
    else {
        QMessageBox::critical(this, "Input Error", "All input fields must be filled out.");
    }
}
