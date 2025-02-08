#ifndef PROFILE_H
#define PROFILE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <curl/curl.h>
#include <QLineEdit>

class Profile : public QWidget
{
    Q_OBJECT

public:
    Profile(int userId, QWidget* parent = nullptr);
    ~Profile();

private:
    int userId;
    QLabel* labelFullName;
    QLabel* labelEmail;
    QLabel* labelRole;
    QLabel* labelPhone;

    QPushButton* editNameButton;
    QPushButton* editEmailButton;
    QPushButton* editPhoneButton;

    QLineEdit* editFullNameField;
    QLineEdit* editEmailField;
    QLineEdit* editPhoneField;

    void fetchProfileData(int userId);
    void parseProfileData(const QString& jsonData);

    void sendUpdateRequest(const QString& field, const QString& newValue);

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

private slots:
    void onEditName();
    void onEditEmail();
    void onEditPhone();
    void toggleEditMode(QLabel* label, QLineEdit* editField, QPushButton* button);
};


#endif 
