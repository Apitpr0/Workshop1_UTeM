#include <iostream>
#include <string>
#include <stdexcept>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>

// Enrollment password for admin registration (change as needed)
const std::string ADMIN_ENROLLMENT_PASSWORD = "admin123";

// Returns true if registration is successful, false otherwise
bool registerUser() {
    std::string username, password, email;
    int role = 0; // 0 = user, 1 = admin

    std::cout << "Register Page\n";
    std::cout << "Register as:\n";
    std::cout << "1. User\n";
    std::cout << "2. Admin\n";
    std::cout << "Enter choice (1 or 2): ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer

    if (choice == 2) {
        std::string enrollmentPassword;
        std::cout << "Enter admin enrollment password: ";
        std::getline(std::cin, enrollmentPassword);
        if (enrollmentPassword != ADMIN_ENROLLMENT_PASSWORD) {
            std::cout << "Invalid enrollment password. Registration aborted.\n";
            return false;
        }
        role = 1;
    } else if (choice != 1) {
        std::cout << "Invalid choice. Registration aborted.\n";
        return false;
    }

    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    std::cout << "Enter email: ";
    std::getline(std::cin, email);

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(
            driver->connect("tcp://localhost:3306", "root", "") 
        );
        con->setSchema("erms"); 

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO users(username, password, email, is_admin) VALUES (?, ?, ?, ?)")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, password);
        pstmt->setString(3, email);
        pstmt->setInt(4, role);

        pstmt->execute();
        std::cout << "Registration successful!\n";
        system("cls");
        return true;
    } catch (sql::SQLException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}