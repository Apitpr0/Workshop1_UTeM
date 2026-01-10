// dashboard.cpp
#include "dashboard.h"
#include "user_dashboard.h"
#include "admin_dashboard.h"
#include "runner_dashboard.h"
#include "loginpage.h"
#include "utils.h" // untuk getMenuChoice dan validation functions
#include "hash.h" // untuk hashPassword
#include <iostream>
#include <limits>
#include <tuple>
#include <string>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

// ===== Forget Password =====
void forgetPassword() {
    clearScreen();
    printMenuTitle("Forget Password");
    
    std::string username;
    while (true) {
        username = getCenteredInput("Enter username (or 'back' to cancel): ");
        if (username == "back" || username == "Back" || username == "BACK") {
            std::cout << "Password reset cancelled.\n";
            return;
        }
        if (username.empty()) {
            std::cout << "Username cannot be empty! Please enter a username.\n";
        }
        else if (username.find(' ') != std::string::npos) {
            std::cout << "Username cannot contain spaces!\n";
        }
        else {
            break;
        }
    }

    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

    // Check if username exists
    sql::PreparedStatement* checkStmt = con->prepareStatement(
        "SELECT user_id, email FROM users WHERE name=?"
    );
    checkStmt->setString(1, username);
    sql::ResultSet* res = checkStmt->executeQuery();

    if (!res->next()) {
        std::cout << "Username not found! Please check your username and try again.\n";
        delete res;
        delete checkStmt;
        delete con;
        return;
    }

    int userId = res->getInt("user_id");
    std::string email = res->getString("email");
    delete res;
    delete checkStmt;

    // Verify email
    std::string inputEmail;
    while (true) {
        inputEmail = getCenteredInput("Enter email to verify (or 'back' to cancel): ");
        if (inputEmail == "back" || inputEmail == "Back" || inputEmail == "BACK") {
            centerText("Password reset cancelled.");
            delete con;
            return;
        }
        if (inputEmail.empty()) {
            centerText("Email cannot be empty! Please enter your email.");
        }
        else if (inputEmail != email) {
            centerText("Email does not match! Please enter the correct email.");
        }
        else {
            break;
        }
    }

    // Get new password
    std::string newPassword;
    while (true) {
        newPassword = getCenteredInput("Enter new password (8+ chars, uppercase, lowercase, number and symbol) or 'back' to cancel: ");
        if (newPassword == "back" || newPassword == "Back" || newPassword == "BACK") {
            centerText("Password reset cancelled.");
            delete con;
            return;
        }
        if (newPassword.empty()) {
            centerText("Password cannot be empty! Please enter a password.");
        }
        else if (!isValidPassword(newPassword)) {
            centerText("Password must be 8+ chars, include uppercase, lowercase, number, and symbol. Re-enter (or 'back' to cancel).");
        }
        else {
            break;
        }
    }

    // Confirm new password
    std::string confirmPassword;
    while (true) {
        confirmPassword = getCenteredInput("Confirm new password (or 'back' to cancel): ");
        if (confirmPassword == "back" || confirmPassword == "Back" || confirmPassword == "BACK") {
            centerText("Password reset cancelled.");
            delete con;
            return;
        }
        if (confirmPassword != newPassword) {
            centerText("Passwords do not match! Re-enter confirmation.");
        }
        else {
            break;
        }
    }

    // Update password
    std::string hashedPassword = hashPassword(newPassword);
    sql::PreparedStatement* updateStmt = con->prepareStatement(
        "UPDATE users SET password=? WHERE user_id=?"
    );
    updateStmt->setString(1, hashedPassword);
    updateStmt->setInt(2, userId);
    int updated = updateStmt->executeUpdate();

    if (updated > 0) {
        printSuccess("Password reset successful! You can now login with your new password.");
    }
    else {
        printError("Failed to reset password. Please try again.");
    }
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    delete updateStmt;
    delete con;
}

// ===== Main dashboard =====
void show_dashboard() {
    while (true) {
        clearScreen();
        
        // Beautiful header with ASCII art style
        std::cout << "\n";
        printMenuTitle("ERMS - Errand Runner Management System, Your trusted Partner");
        printHeader("MAIN MENU");
        std::cout << "\n";
        centerText("+--------------------------------------+");
        centerText("|  1. User Login                      |");
        centerText("|  2. Admin Login                     |");
        centerText("|  3. Runner Login                    |");
        centerText("|  4. Register                        |");
        centerText("|  5. Forget Password                 |");
        centerText("|  0. Exit                            |");
        centerText("+--------------------------------------+");
        std::cout << "\n";
        printHeader("");
        std::cout << "\n";
        centerText("Enter choice: ");
        int choice = getMenuChoice(0, 5);

        switch (choice) {
        case 0:
            return;

        case 1: {
            std::tuple<int, std::string> loginResult = user_login();
            int role = std::get<0>(loginResult);
            std::string username = std::get<1>(loginResult);
            if (role == 0) {
                clearScreen();
                printSuccess("Login successful! Welcome " + username + ".");
                std::cout << "\n";
                user_menu(username);
            }
            else {
                printError("Login failed or wrong role.");
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            break;
        }

        case 2: {
            std::tuple<int, std::string> loginResult = admin_login();
            int role = std::get<0>(loginResult);
            std::string username = std::get<1>(loginResult);
            if (role == 1) {
                clearScreen();
                printSuccess("Login successful! Welcome " + username + ".");
                std::cout << "\n";
                admin_menu(username); 
            }
            else {
                printError("Login failed or wrong role.");
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            break;
        }

        case 3: {
            std::tuple<int, std::string> loginResult = runner_login();
            int role = std::get<0>(loginResult);
            std::string username = std::get<1>(loginResult);

            if (role == 2) {
                clearScreen();
                printSuccess("Login successful! Welcome " + username + ".");
                std::cout << "\n";
                runner_menu(username);
            }
            else {
                printError("Login failed or wrong role.");
                std::cout << "\nPress Enter to continue...";
                std::cin.get();
            }
            break;
        }


        case 4:
            registerUser();
            break;

        case 5:
            forgetPassword();
            break;

        default:
            std::cout << "Invalid choice.\n";
        }
    }
}
