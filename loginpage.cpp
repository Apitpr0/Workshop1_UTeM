#include "loginpage.h"
#include <iostream>
#include <memory>
#include <limits>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "hash.h"
#include "utils.h"

// Helper login function
static std::tuple<int, std::string> loginWithRole(int expectedRole, const std::string& roleName) {
    std::string username, password;

    while (true) {
        clearScreen();
        printMenuTitle(roleName + " Login");
        printInfo("Type 'back' at any time to return to main menu");
        std::cout << "\n";

        // Username input with space check
        while (true) {
            username = getCenteredInput("Username (or 'back' to cancel): ");
            if (username == "back" || username == "Back" || username == "BACK") {
                return { -1, "" }; // Back to main menu
            }
            if (username.empty()) {
                printError("Username cannot be empty!");
            }
            else if (username.find(' ') != std::string::npos) {
                printError("Username cannot contain spaces!");
            }
            else break; // valid username
        }

        // Password input
        password = getCenteredInput("Password (or 'back' to cancel): ");
        if (password == "back" || password == "Back" || password == "BACK") {
            return { -1, "" }; // Back to main menu
        }
        if (password.empty()) {
            printError("Password cannot be empty!");
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            std::cin.get();
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
                    printError("Login failed! This account does not have " + roleName + " access.");
                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                    std::cin.get();
                    return { -1, "" };
                }
                return { role, username };
            }
            else {
                printError("Login failed! Invalid username or password.");
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
                std::cin.get();
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
