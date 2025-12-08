// registerpage.cpp
#include <iostream>
#include <string>
#include <limits>
#include <regex>
#include <memory>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include "hash.h" // your SHA-256 hash function

using namespace std;

// Enrollment passwords
const string ADMIN_ENROLLMENT_PASSWORD = "admin123";
const string RUNNER_ENROLLMENT_PASSWORD = "runner123";

// ===== Validation functions =====
bool isValidEmail(const string& email) {
    const regex pattern(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    return regex_match(email, pattern);
}

bool isValidPhone(const string& phone) {
    const regex pattern(R"(^\d{10,12}$)");
    return regex_match(phone, pattern);
}

bool isValidName(const string& name) {
    const regex pattern(R"(^[A-Za-z ]{2,50}$)");
    return regex_match(name, pattern);
}

bool isValidPassword(const string& pw) {
    if (pw.length() < 8) return false;
    if (!regex_search(pw, regex("[A-Z]"))) return false; // Uppercase
    if (!regex_search(pw, regex("[a-z]"))) return false; // Lowercase
    if (!regex_search(pw, regex("[0-9]"))) return false; // Digit
    if (!regex_search(pw, regex(R"([\W_])"))) return false; // Symbol
    vector<string> blacklist = { "password","12345678","qwerty","admin" };
    for (auto& s : blacklist) if (pw == s) return false;
    return true;
}

// ===== Registration function =====
bool registerUser() {
    string username, password, email, contactNumber;
    int role = 0; // 0=user, 1=admin, 2=runner

    cout << "\n=== Register Page ===\n";
    cout << "Register as:\n1. User\n2. Admin\n3. Runner\nEnter choice: ";
    int choice;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (choice == 2) {
        string enrollmentPassword;
        cout << "Enter admin enrollment password: ";
        getline(cin, enrollmentPassword);
        if (enrollmentPassword != ADMIN_ENROLLMENT_PASSWORD) {
            cout << "Invalid enrollment password. Registration aborted.\n";
            return false;
        }
        role = 1;
    }
    else if (choice == 3) {
        string enrollmentPassword;
        cout << "Enter runner enrollment password: ";
        getline(cin, enrollmentPassword);
        if (enrollmentPassword != RUNNER_ENROLLMENT_PASSWORD) {
            cout << "Invalid enrollment password. Registration aborted.\n";
            return false;
        }
        role = 2;
    }
    else if (choice != 1) {
        cout << "Invalid choice. Registration aborted.\n";
        return false;
    }

    // ===== Username =====
    cout << "Enter username: ";
    getline(cin, username);
    while (!isValidName(username)) {
        cout << "Invalid name! Only letters and spaces allowed (2-50 chars). Re-enter: ";
        getline(cin, username);
    }

    // ===== Password =====
    cout << "Enter password: ";
    getline(cin, password);
    while (!isValidPassword(password)) {
        cout << "Password must be 8+ chars, include uppercase, lowercase, number, and symbol. Re-enter: ";
        getline(cin, password);
    }

    string confirmPassword;
    cout << "Confirm password: ";
    getline(cin, confirmPassword);
    while (confirmPassword != password) {
        cout << "Passwords do not match! Re-enter confirmation: ";
        getline(cin, confirmPassword);
    }

    // ===== Email =====
    cout << "Enter email: ";
    getline(cin, email);
    while (!isValidEmail(email)) {
        cout << "Invalid email format. Re-enter: ";
        getline(cin, email);
    }

    // ===== Contact Number =====
    cout << "Enter contact number: ";
    getline(cin, contactNumber);
    while (!isValidPhone(contactNumber)) {
        cout << "Invalid phone! 10-12 digits only. Re-enter: ";
        getline(cin, contactNumber);
    }

    string hashedPassword = hashPassword(password);

    // ===== Database insertion =====
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        // Check duplicate email
        unique_ptr<sql::PreparedStatement> checkStmt(
            con->prepareStatement("SELECT COUNT(*) AS count FROM users WHERE email=?")
        );
        checkStmt->setString(1, email);
        unique_ptr<sql::ResultSet> res(checkStmt->executeQuery());
        res->next();
        if (res->getInt("count") > 0) {
            cout << "Email already registered! Registration aborted.\n";
            return false;
        }

        unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("INSERT INTO users(name, password, email, c_number, role) VALUES (?, ?, ?, ?, ?)")
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashedPassword);
        pstmt->setString(3, email);
        pstmt->setString(4, contactNumber);
        pstmt->setInt(5, role);
        pstmt->execute();

        cout << "Registration successful!\n";
        return true;
    }
    catch (sql::SQLException& e) {
        cerr << "Database error: " << e.what() << endl;
        return false;
    }
}
