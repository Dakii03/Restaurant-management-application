#ifndef STAFF_MANAGEMENT_H
#define STAFF_MANAGEMENT_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateEdit>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <curl/curl.h>

class StaffManagement : public QWidget
{
    Q_OBJECT

public:
    explicit StaffManagement(QWidget *parent = nullptr);
    ~StaffManagement();

private slots:
    void addStaff();
    void deleteStaff();
    void loadStaffFromDatabase(); 
    void loadRolesFromDatabase();
    QString hashPassword(const QString& password);
    void getNextUserId();
private:
    void addStaffToDatabase(const QString& department, const QString& position, const QString& hireDate); 
    void deleteStaffFromDatabase(int userId); 
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);

    QTableWidget *staffTable;
    QPushButton *addButton;
    QPushButton *deleteButton;

    QLineEdit* usernameEdit;
    QLineEdit* fullNameEdit;
    QLineEdit* emailEdit;
    QLineEdit* phoneEdit;
    QLineEdit* roleEdit;
    QLineEdit* departmentEdit;
    QLineEdit* positionEdit;
    QDateEdit* hireDateEdit;
    QLineEdit* userIdEdit;
    QComboBox* positionComboBox;
};

#endif
