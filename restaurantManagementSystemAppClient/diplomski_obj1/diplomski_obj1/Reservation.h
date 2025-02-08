#ifndef RESERVATION_H
#define RESERVATION_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QGridLayout>
#include <QSpacerItem>
#include <QFont>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include "sqlite3.h"
#include <iostream>
#include <qtimer.h>
#include <qevent.h>
#include <qobject.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <curl/curl.h>
#include <QDateTime>

class Reservation : public QWidget
{
    Q_OBJECT

public:
    explicit Reservation(QWidget* parent = nullptr);
    ~Reservation();
public slots:
    void makeReservation();
    void populateAvailableTables();
    void loadReservationsFromDatabase();  
    void checkForPastReservations();
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);

signals:
    void reservationsLoaded();


private:
    QGridLayout* mainLayout;

    QLabel* nameLabel;
    QLineEdit* nameEdit;

    QLabel* dateLabel;
    QDateEdit* dateEdit;

    QLabel* timeLabel;
    QTimeEdit* timeEdit;

    QLabel* guestsLabel;
    QSpinBox* guestsSpinBox;

    QLabel* tableLabel;
    QComboBox* tableComboBox;

    QPushButton* reserveButton;

    QPushButton* deleteButton;

    QTableWidget* reservationsTable;

    QTimer* updateTimer;

    bool openDatabase();
    void saveReservation(const std::string& name, const QDate& date, const QTime& time, int guests, int tableId);
    void removePastReservations();
    bool checkReservationDate(QDateTime& date);
    void deleteReservation(int reservationId);
};

#endif
