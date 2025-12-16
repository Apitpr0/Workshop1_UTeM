#include <iostream>
#include <string>
#include <memory>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <limits>
#include <iomanip>
#include <cstdlib>
#include <ctime>

// Forward declaration of getMenuChoice from dashboard.cpp
extern int getMenuChoice(int min, int max);

// ===== Get runner ID from username =====
int getRunnerId(const std::string& username) {
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

// ===== View available errands =====
void viewAvailableErrands() {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT errand_id, description, pickup_loc, dropoff_loc, distance "
                "FROM errands WHERE status='Pending' ORDER BY created_at ASC"
            )
        );

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::cout << "\n--- Available Errands ---\n";
        bool hasErrands = false;
        while (res->next()) {
            hasErrands = true;
            std::cout << "ID: " << res->getInt("errand_id")
                << " | Desc: " << res->getString("description")
                << " | Pickup: " << res->getString("pickup_loc")
                << " | Dropoff: " << res->getString("dropoff_loc")
                << " | Distance: " << std::fixed << std::setprecision(2)
                << res->getDouble("distance") << " km\n";
        }
        if (!hasErrands) std::cout << "No available errands at the moment.\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== View errands assigned to runner =====
void viewAssignedErrands(const std::string& username) {
    int runnerId = getRunnerId(username);
    if (runnerId == -1) { std::cout << "Runner not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT errand_id, description, pickup_loc, dropoff_loc, distance, status "
                "FROM errands WHERE runner_id=? ORDER BY created_at DESC"
            )
        );
        pstmt->setInt(1, runnerId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        std::cout << "\n--- My Assigned Errands ---\n";
        bool hasErrands = false;
        while (res->next()) {
            hasErrands = true;
            std::cout << "ID: " << res->getInt("errand_id")
                << " | Desc: " << res->getString("description")
                << " | Pickup: " << res->getString("pickup_loc")
                << " | Dropoff: " << res->getString("dropoff_loc")
                << " | Distance: " << std::fixed << std::setprecision(2)
                << res->getDouble("distance")
                << " km | Status: " << res->getString("status") << "\n";
        }
        if (!hasErrands) std::cout << "No errands assigned to you yet.\n";

    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Accept an errand =====
void acceptErrand(const std::string& username) {
    int runnerId = getRunnerId(username);
    if (runnerId == -1) { std::cout << "Runner not found!\n"; return; }

    viewAvailableErrands();

    int errandId;
    while (true) {
        std::cout << "Enter the ID of the errand to accept: ";
        if (std::cin >> errandId) { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); break; }
        else { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "Invalid input!\n"; }
    }

    char confirm;
    while (true) {
        std::cout << "Confirm acceptance of this errand? (Y/N): ";
        std::cin >> confirm; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (confirm == 'Y' || confirm == 'y' || confirm == 'N' || confirm == 'n') break;
        else std::cout << "Invalid input! Enter Y or N.\n";
    }
    if (confirm == 'N' || confirm == 'n') { std::cout << "Operation canceled.\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "UPDATE errands SET status='Assigned', runner_id=? "
                "WHERE errand_id=? AND status='Pending'"
            )
        );
        pstmt->setInt(1, runnerId);
        pstmt->setInt(2, errandId);

        int updated = pstmt->executeUpdate();
        if (updated > 0) std::cout << "Errand accepted successfully!\n";
        else std::cout << "Errand not found or already assigned.\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Mark an errand as completed =====
void markErrandCompleted(const std::string& username) {
    int runnerId = getRunnerId(username);
    if (runnerId == -1) { std::cout << "Runner not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        // Show errands assigned to this runner and not completed
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT errand_id, description, status FROM errands "
                "WHERE runner_id=? AND status='Assigned' ORDER BY created_at DESC"
            )
        );
        pstmt->setInt(1, runnerId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        bool hasErrands = false;
        std::cout << "\n--- Your Assigned Errands ---\n";
        while (res->next()) {
            hasErrands = true;
            std::cout << "ID: " << res->getInt("errand_id")
                << " | Desc: " << res->getString("description")
                << " | Status: " << res->getString("status") << "\n";
        }
        if (!hasErrands) { std::cout << "No assigned errands to mark as completed.\n"; return; }

        int errandId;
        while (true) {
            std::cout << "Enter the ID of the errand to mark as COMPLETED: ";
            if (std::cin >> errandId) { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); break; }
            else { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "Invalid input!\n"; }
        }

        char confirm;
        while (true) {
            std::cout << "Are you sure you want to mark this errand as COMPLETED? (Y/N): ";
            std::cin >> confirm; std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (confirm == 'Y' || confirm == 'y' || confirm == 'N' || confirm == 'n') break;
            else std::cout << "Invalid input! Enter Y or N.\n";
        }
        if (confirm == 'N' || confirm == 'n') { std::cout << "Operation canceled.\n"; return; }

        std::unique_ptr<sql::PreparedStatement> updateStmt(
            con->prepareStatement("UPDATE errands SET status='Completed' WHERE errand_id=? AND runner_id=? AND status='Assigned'")
        );
        updateStmt->setInt(1, errandId);
        updateStmt->setInt(2, runnerId);

        int updated = updateStmt->executeUpdate();
        if (updated > 0) std::cout << "Errand marked as COMPLETED successfully!\n";
        else std::cout << "Errand not found or not assigned to you!\n";

    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Runner summary stats =====
void viewRunnerStats(const std::string& username) {
    int runnerId = getRunnerId(username);
    if (runnerId == -1) { std::cout << "Runner not found!\n"; return; }

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
                "FROM errands WHERE runner_id=?"
            )
        );
        pstmt->setInt(1, runnerId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (res->next()) {
            std::cout << "\n--- Runner Summary ---\n";
            std::cout << "Total: " << res->getInt("total")
                << " | Pending: " << res->getInt("pending")
                << " | Assigned: " << res->getInt("assigned")
                << " | Completed: " << res->getInt("completed") << "\n";
        }

    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Full runner menu =====
void runner_menu(const std::string& username) {
    while (true) {
        std::cout << "\n=== Runner Dashboard ===\n";
        std::cout << "1. View available errands\n";
        std::cout << "2. View my assigned errands\n";   // NEW
        std::cout << "3. Accept an errand\n";
        std::cout << "4. Mark an errand as completed\n";
        std::cout << "5. View summary stats\n";
        std::cout << "0. Logout\nEnter choice: ";
        int choice = getMenuChoice(0, 5);

        switch (choice) {
        case 0: return;
        case 1: viewAvailableErrands(); break;
        case 2: viewAssignedErrands(username); break; // NEW
        case 3: acceptErrand(username); break;
        case 4: markErrandCompleted(username); break;
        case 5: viewRunnerStats(username); break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}
