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

using namespace std;

// Enrollment passwords
const string ADMIN_ENROLLMENT_PASSWORD = "admin123";
const string RUNNER_ENROLLMENT_PASSWORD = "runner123";

// ===== Validation functions =====
bool isValidEmail(const string& email) {
    const regex pattern(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    return regex_match(email, pattern);
}

bool isValidName(const std::string& u) {
    const std::regex pattern(R"(^[A-Za-z0-9]{2,50}$)"); // 2-50 chars, letters & numbers
    return regex_match(u, pattern);
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

// ===== International phone validation =====
bool isValidPhoneIntl(const std::string& phone) {
    if (phone.empty()) return false;
    if (phone[0] == '+') {
        for (size_t i = 1; i < phone.size(); ++i)
            if (!isdigit(phone[i])) return false;
    }
    else {
        for (char c : phone) if (!isdigit(c)) return false;
    }
    size_t digitsCount = (phone[0] == '+') ? phone.size() - 1 : phone.size();
    return digitsCount >= 7 && digitsCount <= 15;
}

// Normalize phone for duplicate check
std::string normalizePhone(const std::string& input) {
    std::string num;
    for (char c : input) if (isdigit(c)) num += c;
    return num;
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
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        while (true) {
            cout << "Enter username (2-50 chars, letters & numbers, no spaces): ";
            getline(cin, username);

            if (!isValidName(username)) {
                cout << "Invalid username! Re-enter.\n";
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
                cout << "Username already taken! Please enter another.\n";
                continue;
            }

            break; // valid & unique
        }

        // ===== Password =====
        cout << "Enter password (8+ chars, uppercase, lowercase, number and symbol): ";
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
        while (true) {
            cout << "Enter email: ";
            getline(cin, email);

            if (!isValidEmail(email)) {
                cout << "Invalid email format. Re-enter.\n";
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
                cout << "Email already registered! Enter another email.\n";
                continue;
            }

            break; // Valid & unique email
        }


        // ===== Contact Number =====
        while (true) {
            cout << "Enter contact number (with optional +countrycode): ";
            getline(cin, contactNumber);
            if (!isValidPhoneIntl(contactNumber)) {
                cout << "Invalid phone! Must be 7-15 digits, can start with +. Re-enter.\n";
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
                cout << "This contact number is already registered! Enter another number.\n";
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

        cout << "Registration successful!\n";
        return true;
    }
    catch (sql::SQLException& e) {
        cerr << "Database error: " << e.what() << endl;
        return false;
    }
}
