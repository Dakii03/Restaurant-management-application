#ifndef SETUSERNAMEPASSWORDDIALOG_H
#define SETUSERNAMEPASSWORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class SetUsernamePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    SetUsernamePasswordDialog(QWidget* parent = nullptr);
    QString getUsername() const;
    QString getPassword() const;
    QString getFullName() const;
    QString getEmail() const;
    QString getPhone() const;

private:
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* fullNameEdit;
    QLineEdit* emailEdit;
    QLineEdit* phoneEdit;
    QPushButton* okButton;
    QPushButton* cancelButton;

private slots:
    void onOkClicked();
};

#endif
