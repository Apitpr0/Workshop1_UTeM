#include "loginpage.h"
#include <iostream>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "hash.h"

// Helper login function
static std::tuple<int, std::string> loginWithRole(int expectedRole, const std::string& roleName) {
    std::string username, password;

    while (true) {
        std::cout << "\n=== " << roleName << " Login ===\n";

        // Username input with space check
        while (true) {
            std::cout << "Username: ";
            std::getline(std::cin, username);
            if (username.empty()) {
                std::cout << "Username cannot be empty!\n";
            }
            else if (username.find(' ') != std::string::npos) {
                std::cout << "Username cannot contain spaces!\n";
            }
            else break; // valid username
        }

        // Password input
        std::cout << "Password: ";
        std::getline(std::cin, password);
        if (password.empty()) {
            std::cout << "Password cannot be empty!\n";
            continue; // loop back to username/password input
        }

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
                if (role != expectedRole) {
                    std::cout << "Login failed! This account does not have " << roleName << " access.\n";
                    return { -1, "" };
                }
                return { role, username };
            }
            else {
                std::cout << "Login failed! Invalid username or password.\n";
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Database error: " << e.what() << std::endl;
            return { -1, "" };
        }
    }
}

// Separate login functions
std::tuple<int, std::string> user_login() { return loginWithRole(0, "User"); }
std::tuple<int, std::string> admin_login() { return loginWithRole(1, "Admin"); }
std::tuple<int, std::string> runner_login() { return loginWithRole(2, "Runner"); }
