#include <iostream>
#include <string>
#include <memory>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <limits>
#include "hash.h" // for hashPassword

const std::string RUNNER_ENROLLMENT_PASSWORD = "runner123";

// Forward declaration of menu helper
extern int getMenuChoice(int min, int max);

// ===== View all users =====
void viewAllUsers() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT user_id, name, email, c_number, role FROM users")
        );

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::cout << "\n--- All Users ---\n";
        while (res->next()) {
            int role = res->getInt("role");
            std::string roleText = (role == 0) ? "User" : (role == 1) ? "Admin" : "Runner";

            std::cout << "ID: " << res->getInt("user_id")
                << " | Name: " << res->getString("name")
                << " | Email: " << res->getString("email")
                << " | Contact: " << res->getString("c_number")
                << " | Role: " << roleText << "\n";
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}

// ===== View all errands =====
void viewAllErrands() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT e.errand_id, u.name AS requester, e.description, e.pickup_loc, "
                "e.dropoff_loc, e.distance, e.status, e.created_at, r.name AS runner "
                "FROM errands e "
                "LEFT JOIN users u ON e.requester_id = u.user_id "
                "LEFT JOIN users r ON e.runner_id = r.user_id "
                "ORDER BY e.created_at DESC"
            )
        );

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::cout << "\n--- All Errands ---\n";
        while (res->next()) {
            std::string runnerName = res->isNull("runner") ? "Unassigned" : res->getString("runner");

            std::cout << "ID: " << res->getInt("errand_id")
                << " | Requester: " << res->getString("requester")
                << " | Runner: " << runnerName
                << " | Desc: " << res->getString("description")
                << " | Pickup: " << res->getString("pickup_loc")
                << " | Dropoff: " << res->getString("dropoff_loc")
                << " | Distance: " << res->getDouble("distance") << " km"
                << " | Status: " << res->getString("status")
                << " | Created: " << res->getString("created_at") << "\n";
        }
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Update errand status =====
void updateErrandStatusAdmin() {
    viewAllErrands();

    int errandId;
    std::cout << "Enter the ID of the errand to update: ";
    std::cin >> errandId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter new status (Pending, Assigned, Completed): ";
    std::string newStatus;
    std::getline(std::cin, newStatus);

    if (newStatus != "Pending" && newStatus != "Assigned" && newStatus != "Completed") {
        std::cout << "Invalid status!\n";
        return;
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("UPDATE errands SET status=? WHERE errand_id=?")
        );
        pstmt->setString(1, newStatus);
        pstmt->setInt(2, errandId);

        int updated = pstmt->executeUpdate();
        if (updated > 0) std::cout << "Errand status updated successfully!\n";
        else std::cout << "Errand not found!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
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
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "UPDATE errands SET runner_id=?, status='Assigned' WHERE errand_id=?"
            )
        );
        pstmt->setInt(1, runnerId);
        pstmt->setInt(2, errandId);

        int updated = pstmt->executeUpdate();
        if (updated > 0) std::cout << "Errand assigned to runner successfully!\n";
        else std::cout << "Errand or runner not found!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
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
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "INSERT INTO users(name, password, email, c_number, role) VALUES (?, ?, ?, ?, 2)"
            )
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashedPassword);
        pstmt->setString(3, email);
        pstmt->setString(4, contactNumber);
        pstmt->execute();

        std::cout << "Runner registered successfully!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
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
        std::cout << "0. Logout\nEnter choice: ";

        int choice = getMenuChoice(0, 5);
        if (choice == 0) break;

        switch (choice) {
        case 1: viewAllUsers(); break;
        case 2: viewAllErrands(); break;
        case 3: updateErrandStatusAdmin(); break;
        case 4: assignErrandToRunner(); break;
        case 5: registerRunner(); break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}
