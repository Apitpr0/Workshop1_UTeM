#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include "../utils.h" // untuk centerText dan clearScreen
#include <windows.h> // untuk Sleep

void test_db_connection()
{
    clearScreen();
    try {
        sql::Driver* driver = get_driver_instance();
        std::unique_ptr<sql::Connection> con(
            driver->connect("tcp://127.0.0.1:3306", "root", "")
        );
        con->setSchema("erms");
        
        // Beautiful success message
        std::cout << "\n";
        std::cout << "\n";
        centerText("========================================");
        centerText("   [INFO] CONNECTION STATUS [INFO]");
        centerText("========================================");
        centerText("   Database: MySQL");
        centerText("   Status: Connected");
        centerText("========================================");
        std::cout << "\n";
        centerText("Loading dashboard in 3 seconds...");
        std::cout << "\n";
        
        // Wait for 5 seconds (3000 milliseconds)
        Sleep(3000);
        
        // Clear screen before proceeding to dashboard
        clearScreen();
    }
    catch (sql::SQLException& e) {
        std::cout << "\n";
        std::cout << "\n";
        centerText("========================================");
        centerText("   [X] CONNECTION FAILED! [X]");
        centerText("========================================");
        centerText("   Error Message: " + std::string(e.what()));
        centerText("   MySQL Error Code: " + std::to_string(e.getErrorCode()));
        centerText("   SQL State: " + std::string(e.getSQLState()));
        centerText("========================================");
        std::cout << "\n";
        centerText("Press Enter to continue anyway...");
        std::cin.get();
        clearScreen();
    }
}