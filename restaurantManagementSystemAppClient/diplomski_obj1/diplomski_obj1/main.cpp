#include "diplomski_obj1.h"
#include "restaurant_management.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //diplomski_obj1 w;
    restaurant_management w;
    restaurant_management w1;

    w.show();
    w1.show();
    
    return a.exec();
}


//#include <iostream>  
//#include <sqlite3.h>  
//
//int main() {
//    sqlite3* db;
//    char* errMessage = 0;
//
//    // Open database  
//    int exit = sqlite3_open("C:\\Users\\Lenovo\\source\\repos\\diplomski_obj1\\x64\\Release\\restaurant.db", &db);
//    if (exit != SQLITE_OK) {
//        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
//        return -1;
//    }
//
//    // SQL delete statement  
//    const char* sql = "DELETE FROM role_assignments WHERE assignment_id = 6;";
//
//    // Execute SQL statement  
//    exit = sqlite3_exec(db, sql, 0, 0, &errMessage);
//    if (exit != SQLITE_OK) {
//        std::cerr << "Error executing SQL: " << errMessage << std::endl;
//        sqlite3_free(errMessage);
//    }
//    else {
//        std::cout << "Record deleted successfully!" << std::endl;
//    }
//
//    // Close the database  
//    sqlite3_close(db);
//    return 0;
//}

//#include <iostream>  
//#include "sqlite3.h"  
//
//void updateImageUrls(sqlite3* db) {
//    const char* sql = "UPDATE menuitems SET image_url = ?;";
//    sqlite3_stmt* stmt;
//
//    // Prepare the SQL statement  
//    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
//        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
//        return;
//    }
//
//    // Bind the new image URL  
//    const char* newImageUrl = "referenca.jpg";
//    if (sqlite3_bind_text(stmt, 1, newImageUrl, -1, SQLITE_STATIC) != SQLITE_OK) {
//        std::cerr << "Failed to bind value: " << sqlite3_errmsg(db) << std::endl;
//        sqlite3_finalize(stmt);
//        return;
//    }
//
//    // Execute the statement  
//    if (sqlite3_step(stmt) != SQLITE_DONE) {
//        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
//    }
//    else {
//        std::cout << "Image URLs updated successfully." << std::endl;
//    }
//
//    // Finalize the statement to release resources  
//    sqlite3_finalize(stmt);
//}
//
//int main() {
//    sqlite3* db;
//    int rc;
//
//    // Open the database  
//    rc = sqlite3_open("C:\\Users\\Lenovo\\source\\repos\\diplomski_obj1\\x64\\Release\\restaurant.db", &db); // Change to your database file path  
//    if (rc) {
//        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
//        return rc;
//    }
//
//    // Call the update function  
//    updateImageUrls(db);
//
//    // Close database connection  
//    sqlite3_close(db);
//    return 0;
//}

//#include <QCoreApplication>  
//#include <QDebug>  
//#include <sqlite3.h>  
//
//void deleteUserById(const char* dbPath, int userId) {
//    sqlite3* db;
//    char* errorMessage = nullptr;
//
//    // Open the database  
//    int rc = sqlite3_open(dbPath, &db);
//    if (rc) {
//        qDebug() << "Can't open database:" << sqlite3_errmsg(db);
//        return;
//    }
//
//    // Prepare the SQL DELETE statement  
//    const char* sql = "DELETE FROM users WHERE user_id = ?";
//    sqlite3_stmt* stmt;
//
//    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
//    if (rc != SQLITE_OK) {
//        qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(db);
//        sqlite3_close(db);
//        return;
//    }
//
//    // Bind the user_id parameter  
//    sqlite3_bind_int(stmt, 1, userId);
//
//    // Execute the statement  
//    rc = sqlite3_step(stmt);
//    if (rc != SQLITE_DONE) {
//        qDebug() << "Execution failed:" << sqlite3_errmsg(db);
//    }
//    else {
//        if (sqlite3_changes(db) > 0) {
//            qDebug() << "User with id" << userId << "deleted successfully.";
//        }
//        else {
//            qDebug() << "No user found with id" << userId << ".";
//        }
//    }
//
//    // Clean up  
//    sqlite3_finalize(stmt);
//    sqlite3_close(db);
//}
//
//int main(int argc, char* argv[]) {
//    QCoreApplication a(argc, argv);
//
//    // Specify the path to your SQLite database file  
//    const char* dbPath = "C:\\Users\\Lenovo\\source\\repos\\diplomski_obj1\\x64\\Release\\restaurant.db"; // Change to your database path  
//
//    // Call the function to delete user with user_id = 6  
//    deleteUserById(dbPath, 6);
//
//    return a.exec();
//}

//#include <QCryptographicHash>  
//#include <QDebug>  
//#include "sqlite3.h"  
//#include <iostream>
//
//// Function to hash passwords using SHA-256  
//QString hashPassword(const QString& password) {
//    return QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
//}
//
//void addUsers(sqlite3* db) {
//    const char* sql = "INSERT INTO users (username, password_hash, role, full_name, email, is_staff) VALUES (?, ?, ?, ?, ?, ?);";
//    sqlite3_stmt* stmt;
//
//    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
//        qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(db);
//        return;
//    }
//
//    // Administrator (username: admin, password: 1234, full_name: Admin User, email: admin@example.com)  
//    sqlite3_bind_text(stmt, 1, "admin", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 2, hashPassword("1234").toUtf8().constData(), -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 3, "administrator", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 4, "Damjan Jovanvovic", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 5, "admin@example.com", -1, SQLITE_STATIC);
//    sqlite3_bind_int(stmt, 6, 1);  // Bind is_staff as 1 (true) since admin is staff
//    if (sqlite3_step(stmt) != SQLITE_DONE) {
//        qDebug() << "SQL error while adding admin:" << sqlite3_errmsg(db);
//    }
//    sqlite3_reset(stmt);  // Reset the statement to reuse it for the next user  
//
//    // Waiter (username: waiter, password: 1234, full_name: Waiter User, email: waiter@example.com)  
//    sqlite3_bind_text(stmt, 1, "waiter", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 2, hashPassword("1234").toUtf8().constData(), -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 3, "waiter", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 4, "Neki Konobar", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 5, "waiter@example.com", -1, SQLITE_STATIC);
//    sqlite3_bind_int(stmt, 6, 1);  // Bind is_staff as 1 (true)
//    if (sqlite3_step(stmt) != SQLITE_DONE) {
//        qDebug() << "SQL error while adding waiter:" << sqlite3_errmsg(db);
//    }
//    sqlite3_reset(stmt);
//
//    // Manager (username: manager, password: 1234, full_name: Manager User, email: manager@example.com)  
//    sqlite3_bind_text(stmt, 1, "manager", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 2, hashPassword("1234").toUtf8().constData(), -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 3, "manager", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 4, "Neki Menadzer", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 5, "manager@example.com", -1, SQLITE_STATIC);
//    sqlite3_bind_int(stmt, 6, 1);  // Bind is_staff as 1 (true)
//    if (sqlite3_step(stmt) != SQLITE_DONE) {
//        qDebug() << "SQL error while adding manager:" << sqlite3_errmsg(db);
//    }
//    sqlite3_reset(stmt);
//
//    // Kitchen Staff (username: kitchen_stuff, password: 1234, full_name: Kitchen Staff User, email: kitchen@example.com)  
//    sqlite3_bind_text(stmt, 1, "kitchen_stuff", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 2, hashPassword("1234").toUtf8().constData(), -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 3, "kitchen_stuff", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 4, "Kitchen Staff User", -1, SQLITE_STATIC);
//    sqlite3_bind_text(stmt, 5, "kitchen@example.com", -1, SQLITE_STATIC);
//    sqlite3_bind_int(stmt, 6, 1);  // Bind is_staff as 1 (true)
//    if (sqlite3_step(stmt) != SQLITE_DONE) {
//        qDebug() << "SQL error while adding kitchen staff:" << sqlite3_errmsg(db);
//    }
//
//    sqlite3_finalize(stmt);  // Finalize the statement after use  
//}
//
//
//void addPermission(sqlite3* db) {
//    const char* sql = "INSERT INTO permissions (role_id, permission_name) VALUES (?, ?);";
//    sqlite3_stmt* stmt;
//
//    // Prepare the SQL statement  
//    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
//        qDebug() << "Failed to prepare statement:" << sqlite3_errmsg(db);
//        return;
//    }
//
//    // Add permission for role_id 4  
//    sqlite3_bind_int(stmt, 1, 4);  // Role ID  
//    sqlite3_bind_text(stmt, 2, "Manage Customer Order", -1, SQLITE_STATIC);
//
//    if (sqlite3_step(stmt) != SQLITE_DONE) {
//        qDebug() << "SQL error while adding permission for role_id 4:" << sqlite3_errmsg(db);
//    }
//
//    // Finalize the statement to release resources  
//    sqlite3_finalize(stmt);
//}
//
//bool deletePermissionById(sqlite3* db, int permission_id) {
//    // SQL statement to delete the permission by id  
//    std::string sql = "DELETE FROM permissions WHERE permission_id = ?;";
//    sqlite3_stmt* stmt;
//
//    // Prepare the SQL statement  
//    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
//        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
//        return false;
//    }
//
//    // Bind the permission_id variable to the SQL query  
//    sqlite3_bind_int(stmt, 1, permission_id);
//
//    // Execute the statement  
//    if (sqlite3_step(stmt) != SQLITE_DONE) {
//        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
//        sqlite3_finalize(stmt);
//        return false;
//    }
//
//    // Finalize the statement to free resources  
//    sqlite3_finalize(stmt);
//    return true;
//}
//
//int createDatabase(const char* databaseName) {
//    sqlite3* db;
//    char* errMsg = nullptr;
//
//    // Open the database  
//    int rc = sqlite3_open(databaseName, &db);
//    if (rc) {
//        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
//        return rc;
//    }
//
//    const char* sql = R"(  
//    CREATE TABLE customer_order_items (  
//        order_item_id INTEGER PRIMARY KEY AUTOINCREMENT,  -- Unique ID for each order item  
//        order_id INTEGER NOT NULL,                          -- Reference to the order  
//        item_id INTEGER NOT NULL,                           -- Reference to the ordered item  
//        quantity INTEGER NOT NULL,                          -- Quantity of the item ordered  
//        FOREIGN KEY (order_id) REFERENCES customer_orders(id), -- Link to customer_orders  
//        FOREIGN KEY (item_id) REFERENCES menuitems(item_id)  -- Link to menuitems  
//    );    
//)";
//
//    // Execute SQL statement  
//    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
//    if (rc != SQLITE_OK) {
//        std::cerr << "SQL error: " << errMsg << std::endl;
//        sqlite3_free(errMsg);
//    }
//    else {
//        std::cout << "Table created successfully." << std::endl;
//    }
//
//    // Close the database  
//    sqlite3_close(db);
//    return 0;
//}
//
//int dropCustomerOrdersTable(const char* databaseName) {
//    sqlite3* db;
//    char* errMsg = nullptr;
//
//    // Open the database  
//    int rc = sqlite3_open(databaseName, &db);
//    if (rc) {
//        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
//        return rc;
//    }
//
//    // SQL statement to drop the customer_orders table  
//    const char* sql = "DROP TABLE IF EXISTS customer_order_items;";
//
//    // Execute SQL statement  
//    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
//    if (rc != SQLITE_OK) {
//        std::cerr << "SQL error: " << errMsg << std::endl;
//        sqlite3_free(errMsg);
//    }
//    else {
//        std::cout << "Table customer_orders dropped successfully." << std::endl;
//    }
//
//    // Close the database  
//    sqlite3_close(db);
//    return 0;
//}
//
//int main() {
//    //sqlite3* db;
//    const char* dbPath = "C:\\Users\\Lenovo\\source\\repos\\diplomski_obj1\\x64\\Release\\restaurant.db";  // Update the database path  
//
//    //if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
//    //    qDebug() << "Failed to open database:" << sqlite3_errmsg(db);
//    //    return -1;
//    //}
//
//    //addUsers(db);
//    //addPermission(db);
//    //deletePermissionById(db, 10);
//    //createDatabase(dbPath);
//    //dropCustomerOrdersTable(dbPath);
//    //sqlite3_close(db);
//    return 0;
//}



//#include <iostream>
//#include "sqlite3.h"
//
//void executeSQL(sqlite3* db, const std::string& sql) {
//    char* errorMessage = nullptr;
//
//    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errorMessage);
//    if (rc != SQLITE_OK) {
//        std::cerr << "SQL error: " << errorMessage << std::endl;
//        sqlite3_free(errorMessage);
//    }
//    else {
//        std::cout << "Operation done successfully." << std::endl;
//    }
//}
//
//int main() {
//    sqlite3* db;
//    int rc = sqlite3_open("C:\\Users\\Lenovo\\source\\repos\\diplomski_obj1\\x64\\Release\\restaurant.db", &db); // Replace with your database name  
//
//    if (rc) {
//        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
//        return rc;
//    }
//
//    // SQL commands to delete everything from the tables  
//    std::string sqlDeleteCarts = "DELETE FROM carts;";
//    std::string sqlDeleteCartItems = "DELETE FROM cart_items;";
//
//    // Execute the deletion statements  
//    executeSQL(db, sqlDeleteCarts);
//    executeSQL(db, sqlDeleteCartItems);
//
//    // Close the database connection  
//    sqlite3_close(db);
//
//    return 0;
//}