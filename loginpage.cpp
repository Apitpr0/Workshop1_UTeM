// loginpage.cpp
#include <iostream>
#include <string>
#include <memory>
#include <tuple>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "hash.h"

// Returns {role, username}: role = 0=user, 1=admin, -1=failed
std::tuple<int, std::string> show_login_page() {
    std::string username, password;

    std::cout << "\n=== Login Page ===\n";
    std::cout << "Username: ";
    std::getline(std::cin, username);
    std::cout << "Password: ";
    std::getline(std::cin, password);

    std::string hashedInput = hashPassword(password);

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT role FROM users WHERE name=? AND password=?")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashedInput);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            int role = res->getInt("role");
            std::cout << "Login successful! Welcome, " << username << ".\n";
            return { role, username };
        }
        else {
            std::cout << "Login failed! Invalid username or password.\n";
            return { -1, "" };
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return { -1, "" };
    }
}
