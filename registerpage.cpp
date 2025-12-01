// registerpage.cpp
#include <iostream>
#include <string>
#include <limits>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include "hash.h"

const std::string ADMIN_ENROLLMENT_PASSWORD = "admin123";

bool registerUser() {
    std::string username, password, email, contactNumber;
    int role = 0;

    std::cout << "\n=== Register Page ===\n";
    std::cout << "Register as:\n1. User\n2. Admin\nEnter choice: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice == 2) {
        std::string enrollmentPassword;
        std::cout << "Enter admin enrollment password: ";
        std::getline(std::cin, enrollmentPassword);
        if (enrollmentPassword != ADMIN_ENROLLMENT_PASSWORD) {
            std::cout << "Invalid enrollment password. Registration aborted.\n";
            return false;
        }
        role = 1;
    }
    else if (choice != 1) {
        std::cout << "Invalid choice. Registration aborted.\n";
        return false;
    }

    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    std::cout << "Enter email: ";
    std::getline(std::cin, email);
    std::cout << "Enter contact number: ";
    std::getline(std::cin, contactNumber);

    std::string hashedPassword = hashPassword(password);

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO users(name, password, email, c_number, role) VALUES (?, ?, ?, ?, ?)")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashedPassword);
        pstmt->setString(3, email);
        pstmt->setString(4, contactNumber);
        pstmt->setInt(5, role);
        pstmt->execute();

        std::cout << "Registration successful!\n";
        return true;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return false;
    }
}
