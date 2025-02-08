#ifndef TABLE_MANAGEMENT_H
#define TABLE_MANAGEMENT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QGridLayout>
#include <sqlite3.h>
#include <QMessageBox>
#include <curl/curl.h>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class TableManagement : public QWidget
{
    Q_OBJECT

public:
    explicit TableManagement(QWidget* parent = nullptr);
    ~TableManagement();
    void refreshTables();


public slots:
    void handleTableClick(int tableNumber);
    void releaseTable();
    void handlePaymentFinalization(int tableNumber);
    int* checkReservedTables();
    int* checkOccupiedTables();
    void createTableOverview();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    void releaseAllTables(); 

signals:
    void tableStatusChanged();  

private:
    sqlite3* db;
    QMap<int, QWidget*> tableWidgets;
    QVBoxLayout* mainLayout;
    QLabel* tableDetailsLabel;
    QPushButton* tables[10];    
    QPushButton* releaseButton;
    QGroupBox* tableOverviewGroupBox;
    int currentTableNumber;

    void updateTableStatus(int tableNumber, const QString& status);

    //void initializeTablesInDatabase();
    void updateTableOccupiedStatusInDatabase(int tableNumber, bool isOccupied);
};

#endif
