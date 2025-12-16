#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include "hash.h" // untuk hashPassword

const std::string RUNNER_ENROLLMENT_PASSWORD = "runner123";

// Forward declaration
int getMenuChoice(int min, int max);

// Helper untuk truncate string panjang
std::string truncateString(const std::string& str, size_t width) {
    if (str.length() <= width) return str;
    if (width <= 3) return str.substr(0, width);
    return str.substr(0, width - 3) + "...";
}

// ===== View all users =====
void viewAllUsers() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "SELECT user_id, name, email, c_number, role FROM users"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        std::cout << "\n--- All Users ---\n";
        std::cout << std::left
            << std::setw(5) << "ID"
            << std::setw(20) << "Name"
            << std::setw(25) << "Email"
            << std::setw(15) << "Contact"
            << std::setw(10) << "Role" << "\n";
        std::cout << std::string(75, '-') << "\n";

        while (res->next()) {
            int role = res->getInt("role");
            std::string roleText = (role == 0) ? "User" : (role == 1) ? "Admin" : "Runner";

            std::cout << std::left
                << std::setw(5) << res->getInt("user_id")
                << std::setw(20) << truncateString(res->getString("name"), 20)
                << std::setw(25) << truncateString(res->getString("email"), 25)
                << std::setw(15) << truncateString(res->getString("c_number"), 15)
                << std::setw(10) << truncateString(roleText, 10)
                << "\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << "\n";
    }
}

// ===== View all errands =====
void viewAllErrands() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "SELECT e.errand_id, u.name AS requester, e.description, e.pickup_loc, "
            "e.dropoff_loc, e.distance, e.status, e.created_at, r.name AS runner "
            "FROM errands e "
            "LEFT JOIN users u ON e.requester_id = u.user_id "
            "LEFT JOIN users r ON e.runner_id = r.user_id "
            "ORDER BY e.created_at DESC"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        std::cout << "\n--- All Errands ---\n";
        std::cout << std::left
            << std::setw(5) << "ID"
            << std::setw(15) << "Requester"
            << std::setw(15) << "Runner"
            << std::setw(25) << "Description"
            << std::setw(15) << "Pickup"
            << std::setw(15) << "Dropoff"
            << std::setw(10) << "Distance"
            << std::setw(12) << "Status"
            << std::setw(20) << "Created At" << "\n";
        std::cout << std::string(130, '-') << "\n";

        while (res->next()) {
            std::string runnerName = res->isNull("runner") ? "Unassigned" : res->getString("runner");

            std::cout << std::left
                << std::setw(5) << res->getInt("errand_id")
                << std::setw(15) << truncateString(res->getString("requester"), 15)
                << std::setw(15) << truncateString(runnerName, 15)
                << std::setw(25) << truncateString(res->getString("description"), 25)
                << std::setw(15) << truncateString(res->getString("pickup_loc"), 15)
                << std::setw(15) << truncateString(res->getString("dropoff_loc"), 15)
                << std::setw(10) << res->getDouble("distance")
                << std::setw(12) << truncateString(res->getString("status"), 12)
                << std::setw(20) << truncateString(res->getString("created_at"), 20)
                << "\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << "\n";
    }
}

// ===== Update errand status =====
void updateErrandStatusAdmin() {
    viewAllErrands();
    int errandId;
    std::cout << "Enter the ID of the errand to update: ";
    std::cin >> errandId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string newStatus;
    std::cout << "Enter new status (Pending, Assigned, Completed): ";
    std::getline(std::cin, newStatus);

    if (newStatus != "Pending" && newStatus != "Assigned" && newStatus != "Completed") {
        std::cout << "Invalid status!\n";
        return;
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE errands SET status=? WHERE errand_id=?"
        );
        pstmt->setString(1, newStatus);
        pstmt->setInt(2, errandId);
        int updated = pstmt->executeUpdate();
        if (updated > 0) std::cout << "Errand status updated successfully!\n";
        else std::cout << "Errand not found!\n";

        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << "\n";
    }

    viewAllErrands();
}

// ===== Assign errand to runner =====
void assignErrandToRunner() {
    viewAllErrands();
    int errandId, runnerId;
    std::cout << "Enter the ID of the errand to assign: ";
    std::cin >> errandId;
    std::cout << "Enter the runner's user ID: ";
    std::cin >> runnerId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE errands SET runner_id=?, status='Assigned' WHERE errand_id=?"
        );
        pstmt->setInt(1, runnerId);
        pstmt->setInt(2, errandId);
        int updated = pstmt->executeUpdate();
        if (updated > 0) std::cout << "Errand assigned to runner successfully!\n";
        else std::cout << "Errand or runner not found!\n";

        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << "\n";
    }

    viewAllErrands();
}

// ===== Register runner manually =====
void registerRunner() {
    std::string enrollmentPassword;
    std::cout << "Enter runner enrollment password: ";
    std::getline(std::cin, enrollmentPassword);
    if (enrollmentPassword != RUNNER_ENROLLMENT_PASSWORD) {
        std::cout << "Invalid enrollment password. Registration aborted.\n";
        return;
    }

    std::string username, password, email, contactNumber;
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
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "INSERT INTO users(name, password, email, c_number, role) VALUES (?, ?, ?, ?, 2)"
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashedPassword);
        pstmt->setString(3, email);
        pstmt->setString(4, contactNumber);
        pstmt->execute();

        std::cout << "Runner registered successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << "\n";
    }
}

// ===== Reporting Module =====
void reportingModule() {
    while (true) {
        std::cout << "\n=== Reporting Module ===\n";
        std::cout << "1. User Activity Report\n";
        std::cout << "2. Runner Activity Report\n";
        std::cout << "3. Errand Report\n";
        std::cout << "4. Trend / Statistics\n";
        std::cout << "5. General / Combined Report\n";
        std::cout << "0. Back to Admin Dashboard\nEnter choice: ";

        int choice = getMenuChoice(0, 5);
        if (choice == 0) break;

        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
            con->setSchema("erms");

            if (choice == 1) {
                // User Activity Report
                sql::PreparedStatement* pstmt = con->prepareStatement(
                    "SELECT u.user_id, u.name, COUNT(e.errand_id) AS total_errands, "
                    "SUM(e.status='Pending') AS pending, SUM(e.status='Assigned') AS assigned, SUM(e.status='Completed') AS completed "
                    "FROM users u LEFT JOIN errands e ON u.user_id=e.requester_id "
                    "WHERE u.role=0 GROUP BY u.user_id ORDER BY u.name"
                );
                sql::ResultSet* res = pstmt->executeQuery();
                std::cout << "\n--- User Activity Report ---\n";
                std::cout << std::left << std::setw(5) << "ID"
                    << std::setw(20) << "Name"
                    << std::setw(8) << "Total"
                    << std::setw(8) << "Pending"
                    << std::setw(8) << "Assigned"
                    << std::setw(8) << "Completed" << "\n";
                std::cout << std::string(60, '-') << "\n";

                while (res->next()) {
                    std::cout << std::setw(5) << res->getInt("user_id")
                        << std::setw(20) << truncateString(res->getString("name"), 20)
                        << std::setw(8) << res->getInt("total_errands")
                        << std::setw(8) << res->getInt("pending")
                        << std::setw(8) << res->getInt("assigned")
                        << std::setw(8) << res->getInt("completed")
                        << "\n";
                }

                delete res;
                delete pstmt;
            }
            else if (choice == 2) {
                // Runner Activity Report
                sql::PreparedStatement* pstmt = con->prepareStatement(
                    "SELECT u.user_id, u.name, COUNT(e.errand_id) AS total_errands, "
                    "SUM(e.status='Pending') AS pending, SUM(e.status='Assigned') AS assigned, SUM(e.status='Completed') AS completed "
                    "FROM users u LEFT JOIN errands e ON u.user_id=e.runner_id "
                    "WHERE u.role=2 GROUP BY u.user_id ORDER BY u.name"
                );
                sql::ResultSet* res = pstmt->executeQuery();
                std::cout << "\n--- Runner Activity Report ---\n";
                std::cout << std::left << std::setw(5) << "ID"
                    << std::setw(20) << "Name"
                    << std::setw(8) << "Total"
                    << std::setw(8) << "Pending"
                    << std::setw(8) << "Assigned"
                    << std::setw(8) << "Completed" << "\n";
                std::cout << std::string(60, '-') << "\n";

                while (res->next()) {
                    std::cout << std::setw(5) << res->getInt("user_id")
                        << std::setw(20) << truncateString(res->getString("name"), 20)
                        << std::setw(8) << res->getInt("total_errands")
                        << std::setw(8) << res->getInt("pending")
                        << std::setw(8) << res->getInt("assigned")
                        << std::setw(8) << res->getInt("completed")
                        << "\n";
                }

                delete res;
                delete pstmt;
            }
            else if (choice == 3) {
                // Errand Report
                viewAllErrands();
            }
            else if (choice == 4) {
                // Trend / Statistics
                sql::PreparedStatement* pstmt = con->prepareStatement(
                    "SELECT DATE(created_at) AS date, COUNT(*) AS total_errands, "
                    "SUM(status='Pending') AS pending, SUM(status='Assigned') AS assigned, SUM(status='Completed') AS completed "
                    "FROM errands GROUP BY DATE(created_at) ORDER BY DATE(created_at)"
                );
                sql::ResultSet* res = pstmt->executeQuery();
                std::cout << "\n--- Trend / Statistics ---\n";
                std::cout << std::left
                    << std::setw(12) << "Date"
                    << std::setw(8) << "Total"
                    << std::setw(8) << "Pending"
                    << std::setw(8) << "Assigned"
                    << std::setw(8) << "Completed" << "\n";
                std::cout << std::string(44, '-') << "\n";

                while (res->next()) {
                    std::cout << std::setw(12) << truncateString(res->getString("date"), 12)
                        << std::setw(8) << res->getInt("total_errands")
                        << std::setw(8) << res->getInt("pending")
                        << std::setw(8) << res->getInt("assigned")
                        << std::setw(8) << res->getInt("completed") << "\n";
                }

                delete res;
                delete pstmt;
            }
            else if (choice == 5) {
                // General / Combined Report
                viewAllUsers();
                viewAllErrands();
            }

            delete con;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Database error: " << e.what() << "\n";
        }
    }
}

// ===== Admin menu =====
void admin_menu(const std::string& adminUsername) {
    while (true) {
        std::cout << "\n=== Admin Dashboard ===\n";
        std::cout << "1. View all users\n";
        std::cout << "2. View all errands\n";
        std::cout << "3. Update errand status\n";
        std::cout << "4. Assign errand to runner\n";
        std::cout << "5. Register runner manually\n";
        std::cout << "6. Reporting Module\n";
        std::cout << "0. Logout\nEnter choice: ";

        int choice = getMenuChoice(0, 6);
        if (choice == 0) break;

        switch (choice) {
        case 1: viewAllUsers(); break;
        case 2: viewAllErrands(); break;
        case 3: updateErrandStatusAdmin(); break;
        case 4: assignErrandToRunner(); break;
        case 5: registerRunner(); break;
        case 6: reportingModule(); break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}
