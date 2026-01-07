// registerpage.cpp
#include <iostream>
#include <string>
#include <limits>
#include <regex>
#include <memory>
#include <vector>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "hash.h"
#include "utils.h" // untuk validation functions dan UI functions

using namespace std;

// Enrollment passwords
const string ADMIN_ENROLLMENT_PASSWORD = "admin123";
const string RUNNER_ENROLLMENT_PASSWORD = "runner123";

// ===== Registration function =====
bool registerUser() {
    string username, password, email, contactNumber;
    int role = 0; // 0=user, 1=admin, 2=runner

    clearScreen();
    printMenuTitle("Register Page");
    printHeader("REGISTER AS");
    centerText("1. User");
    centerText("2. Admin");
    centerText("3. Runner");
    centerText("0. Back to main menu");
    printHeader("");
    std::cout << "\n";
    centerText("Enter choice: ");
    int choice;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 0) {
        printInfo("Registration cancelled.");
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        return false;
    }

    if (choice == 2) {
        string enrollmentPassword;
        enrollmentPassword = getCenteredInput("Enter admin enrollment password (or 'back' to cancel): ");
        if (enrollmentPassword == "back" || enrollmentPassword == "Back" || enrollmentPassword == "BACK") {
            centerText("Registration cancelled.");
            return false;
        }
        if (enrollmentPassword != ADMIN_ENROLLMENT_PASSWORD) {
            centerText("Invalid enrollment password. Registration aborted.");
            return false;
        }
        role = 1;
    }
    else if (choice == 3) {
        string enrollmentPassword;
        enrollmentPassword = getCenteredInput("Enter runner enrollment password (or 'back' to cancel): ");
        if (enrollmentPassword == "back" || enrollmentPassword == "Back" || enrollmentPassword == "BACK") {
            centerText("Registration cancelled.");
            return false;
        }
        if (enrollmentPassword != RUNNER_ENROLLMENT_PASSWORD) {
            centerText("Invalid enrollment password. Registration aborted.");
            return false;
        }
        role = 2;
    }
    else if (choice != 1) {
        centerText("Invalid choice. Registration aborted.");
        return false;
    }

    // ===== Username =====
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        while (true) {
            username = getCenteredInput("Enter username (2-50 chars, letters & numbers, no spaces) or 'back' to cancel: ");

            if (username == "back" || username == "Back" || username == "BACK") {
                centerText("Registration cancelled.");
                return false;
            }

            if (!isValidName(username)) {
                centerText("Invalid username! Re-enter.");
                continue;
            }

            // Check duplicate username
            unique_ptr<sql::PreparedStatement> checkUserStmt(
                con->prepareStatement("SELECT COUNT(*) AS count FROM users WHERE name=?")
            );
            checkUserStmt->setString(1, username);
            unique_ptr<sql::ResultSet> resUser(checkUserStmt->executeQuery());
            resUser->next();
            if (resUser->getInt("count") > 0) {
                centerText("Username already taken! Please enter another.");
                continue;
            }

            break; // valid & unique
        }

        // ===== Password =====
        password = getCenteredInput("Enter password (8+ chars, uppercase, lowercase, number and symbol) or 'back' to cancel: ");
        if (password == "back" || password == "Back" || password == "BACK") {
            centerText("Registration cancelled.");
            return false;
        }
        while (!isValidPassword(password)) {
            centerText("Password must be 8+ chars, include uppercase, lowercase, number, and symbol. Re-enter (or 'back' to cancel).");
            password = getCenteredInput("Enter password (or 'back' to cancel): ");
            if (password == "back" || password == "Back" || password == "BACK") {
                centerText("Registration cancelled.");
                return false;
            }
        }

        string confirmPassword;
        confirmPassword = getCenteredInput("Confirm password (or 'back' to cancel): ");
        if (confirmPassword == "back" || confirmPassword == "Back" || confirmPassword == "BACK") {
            centerText("Registration cancelled.");
            return false;
        }
        while (confirmPassword != password) {
            centerText("Passwords do not match! Re-enter confirmation (or 'back' to cancel).");
            confirmPassword = getCenteredInput("Confirm password (or 'back' to cancel): ");
            if (confirmPassword == "back" || confirmPassword == "Back" || confirmPassword == "BACK") {
                centerText("Registration cancelled.");
                return false;
            }
        }

        // ===== Email =====
        while (true) {
            email = getCenteredInput("Enter email (or 'back' to cancel): ");

            if (email == "back" || email == "Back" || email == "BACK") {
                centerText("Registration cancelled.");
                return false;
            }

            if (!isValidEmail(email)) {
                centerText("Invalid email format. Re-enter.");
                continue;
            }

            // Check duplicate email
            unique_ptr<sql::PreparedStatement> checkEmailStmt(
                con->prepareStatement("SELECT COUNT(*) AS count FROM users WHERE email=?")
            );
            checkEmailStmt->setString(1, email);
            unique_ptr<sql::ResultSet> resEmail(checkEmailStmt->executeQuery());
            resEmail->next();
            if (resEmail->getInt("count") > 0) {
                centerText("Email already registered! Enter another email.");
                continue;
            }

            break; // Valid & unique email
        }


        // ===== Contact Number =====
        while (true) {
            contactNumber = getCenteredInput("Enter contact number (with optional +countrycode, or 'back' to cancel): ");
            
            if (contactNumber == "back" || contactNumber == "Back" || contactNumber == "BACK") {
                centerText("Registration cancelled.");
                return false;
            }
            
            if (!isValidPhoneIntl(contactNumber)) {
                centerText("Invalid phone! Must be 7-15 digits, can start with +. Re-enter.");
                continue;
            }

            // Check duplicate
            unique_ptr<sql::PreparedStatement> checkStmt(
                con->prepareStatement("SELECT c_number FROM users")
            );
            unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());
            bool duplicate = false;
            string normalizedInput = normalizePhone(contactNumber);
            while (res->next()) {
                string existing = normalizePhone(res->getString("c_number"));
                if (normalizedInput == existing) { duplicate = true; break; }
            }

            if (duplicate) {
                centerText("This contact number is already registered! Enter another number.");
                continue;
            }

            break; // Valid & unique
        }

        string hashedPassword = hashPassword(password);

        // Insert user
        unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO users(name, password, email, c_number, role) VALUES (?, ?, ?, ?, ?)")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashedPassword);
        pstmt->setString(3, email);
        pstmt->setString(4, contactNumber);
        pstmt->setInt(5, role);
        pstmt->execute();

        printSuccess("Registration successful!");
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        return true;
    }
    catch (sql::SQLException& e) {
        printError("Database error: " + string(e.what()));
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        return false;
    }
}
