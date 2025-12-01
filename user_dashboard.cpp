// user_dashboard.cpp
#include <iostream>
#include <string>
#include <memory>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <iomanip>  // for formatting

// Forward declaration of getMenuChoice from dashboard.cpp
extern int getMenuChoice(int min, int max);

// ===== Get user ID from username =====
int getUserId(const std::string& username) {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement("SELECT user_id FROM users WHERE name=?")
        );
        pstmt->setString(1, username);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) return res->getInt("user_id");
        else return -1;
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        return -1;
    }
}

// ===== View errands =====
void viewMyErrands(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) {
        std::cout << "User not found!\n";
        return;
    }

    std::cout << "\nFilter errands by status:\n";
    std::cout << "1. All\n2. Pending\n3. Assigned\n4. Completed\nEnter choice: ";
    int filterChoice = getMenuChoice(1, 4);
    std::string statusFilter = "";
    if (filterChoice == 2) statusFilter = "Pending";
    else if (filterChoice == 3) statusFilter = "Assigned";
    else if (filterChoice == 4) statusFilter = "Completed";

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt;
        if (statusFilter.empty()) {
            pstmt.reset(con->prepareStatement(
                "SELECT errand_id, description, pickup_loc, dropoff_loc, distance, status, created_at "
                "FROM errands WHERE requester_id=? ORDER BY created_at DESC"
            ));
        }
        else {
            pstmt.reset(con->prepareStatement(
                "SELECT errand_id, description, pickup_loc, dropoff_loc, distance, status, created_at "
                "FROM errands WHERE requester_id=? AND status=? ORDER BY created_at DESC"
            ));
            pstmt->setString(2, statusFilter);
        }
        pstmt->setInt(1, userId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::cout << "\n--- Your Errands ---\n";
        bool hasErrands = false;
        while (res->next()) {
            hasErrands = true;
            std::cout << "ID: " << res->getInt("errand_id")
                << " | Desc: " << res->getString("description")
                << " | Pickup: " << res->getString("pickup_loc")
                << " | Dropoff: " << res->getString("dropoff_loc")
                << " | Distance: " << std::fixed << std::setprecision(2) << res->getDouble("distance") << " km"
                << " | Status: " << res->getString("status")
                << " | Created: " << res->getString("created_at") << "\n";
        }
        if (!hasErrands) std::cout << "No errands found!\n";
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}

// ===== Create new errand =====
void createNewErrand(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    std::string desc, pickup, dropoff;
    std::cout << "Enter errand description: "; std::getline(std::cin, desc);
    std::cout << "Enter pickup location: "; std::getline(std::cin, pickup);
    std::cout << "Enter dropoff location: "; std::getline(std::cin, dropoff);

    std::srand(static_cast<unsigned>(std::time(nullptr)));
    double distance = 1.0 + static_cast<double>(std::rand()) / RAND_MAX * 19.0;

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "INSERT INTO errands(requester_id, description, pickup_loc, dropoff_loc, distance) "
                "VALUES(?, ?, ?, ?, ?)"
            )
        );
        pstmt->setInt(1, userId);
        pstmt->setString(2, desc);
        pstmt->setString(3, pickup);
        pstmt->setString(4, dropoff);
        pstmt->setDouble(5, distance);
        pstmt->execute();

        std::cout << "New errand added successfully! Distance: " << distance << " km\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Mark errand as completed =====
void updateErrandStatus(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    // Show all errands first
    viewMyErrands(username);

    int errandId;
    std::cout << "Enter the ID of the errand to mark as COMPLETED: ";
    std::cin >> errandId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    char confirm;
    std::cout << "Are you sure you want to mark this errand as COMPLETED? (Y/N): ";
    std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (confirm != 'Y' && confirm != 'y') { std::cout << "Operation canceled.\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "UPDATE errands SET status='Completed' WHERE requester_id=? AND errand_id=?"
            )
        );
        pstmt->setInt(1, userId);
        pstmt->setInt(2, errandId);

        int updated = pstmt->executeUpdate();
        if (updated > 0) std::cout << "Errand marked as COMPLETED successfully!\n";
        else std::cout << "Errand not found or not linked to you!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Cancel pending errand =====
void cancelPendingErrand(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT errand_id, description FROM errands WHERE requester_id=? AND status='Pending'"
            )
        );
        pstmt->setInt(1, userId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::cout << "\n--- Pending Errands ---\n";
        bool hasPending = false;
        while (res->next()) {
            hasPending = true;
            std::cout << "ID: " << res->getInt("errand_id")
                << " | Description: " << res->getString("description") << "\n";
        }
        if (!hasPending) { std::cout << "No pending errands to cancel.\n"; return; }
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; return; }

    int errandId;
    std::cout << "Enter the ID of the errand to cancel: ";
    std::cin >> errandId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    char confirm;
    std::cout << "Are you sure you want to CANCEL this errand? (Y/N): ";
    std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (confirm != 'Y' && confirm != 'y') { std::cout << "Operation canceled.\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "DELETE FROM errands WHERE requester_id=? AND errand_id=? AND status='Pending'"
            )
        );
        pstmt->setInt(1, userId);
        pstmt->setInt(2, errandId);

        int deleted = pstmt->executeUpdate();
        if (deleted > 0) std::cout << "Errand canceled successfully!\n";
        else std::cout << "Errand not found, not pending, or not linked to you!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Summary stats =====
void viewSummaryStats(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT COUNT(*) AS total, "
                "SUM(status='Pending') AS pending, "
                "SUM(status='Assigned') AS assigned, "
                "SUM(status='Completed') AS completed "
                "FROM errands WHERE requester_id=?"
            )
        );
        pstmt->setInt(1, userId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            std::cout << "\n--- Errand Summary ---\n";
            std::cout << "Total: " << res->getInt("total")
                << " | Pending: " << res->getInt("pending")
                << " | Assigned: " << res->getInt("assigned")
                << " | Completed: " << res->getInt("completed") << "\n";
        }
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== User menu (fully featured) =====
void user_menu(const std::string& username) {
    while (true) {
        std::cout << "\n=== User Dashboard ===\n";
        std::cout << "1. View my errands (with filter)\n";
        std::cout << "2. Create new errand\n";
        std::cout << "3. Mark an errand as completed\n";
        std::cout << "4. Cancel pending errand\n";
        std::cout << "5. View summary stats\n";
        std::cout << "0. Logout\nEnter choice: ";
        int choice = getMenuChoice(0, 5);

        if (choice == 0) break;
        switch (choice) {
        case 1: viewMyErrands(username); break;
        case 2: createNewErrand(username); break;
        case 3: updateErrandStatus(username); break;
        case 4: cancelPendingErrand(username); break;
        case 5: viewSummaryStats(username); break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}
