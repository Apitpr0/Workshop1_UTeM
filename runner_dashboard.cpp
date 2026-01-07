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
#include <sstream>
#include "utils.h"  // untuk getMenuChoice dan UI functions

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

        // Ambik semua pending errands + join quotations (left join untuk fallback)
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT e.errand_id, e.description, e.pickup_loc, e.dropoff_loc, e.distance, "
                "q.runner_share, q.base_price_per_km, q.runner_percentage "
                "FROM errands e "
                "LEFT JOIN quotations q ON e.errand_id = q.errand_id "
                "WHERE e.status='Pending' "
                "ORDER BY e.created_at ASC"
            )
        );

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        
        clearScreen();
        printMenuTitle("Available Errands");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Description", 25},
            {"Pickup", 15},
            {"Dropoff", 15},
            {"Distance", 10},
            {"Runner Earn", 12}
        };
        std::vector<int> widths = {5, 25, 15, 15, 10, 12};
        
        printTableHeader(columns);
        
        bool hasErrands = false;
        while (res->next()) {
            hasErrands = true;
            int errandId = res->getInt("errand_id");
            std::string desc = res->getString("description");
            std::string pickup = res->getString("pickup_loc");
            std::string dropoff = res->getString("dropoff_loc");
            double distance = res->getDouble("distance");

            double runnerShare = 0.0;
            if (!res->isNull("runner_share")) {
                runnerShare = res->getDouble("runner_share");
            }
            else {
                double basePricePerKm = !res->isNull("base_price_per_km") ? res->getDouble("base_price_per_km") : 3.0;
                double runnerPercentage = !res->isNull("runner_percentage") ? res->getDouble("runner_percentage") : 70.0;
                runnerShare = distance * basePricePerKm * (runnerPercentage / 100.0);
            }

            std::ostringstream distStream, earnStream;
            distStream << std::fixed << std::setprecision(2) << distance;
            earnStream << std::fixed << std::setprecision(2) << runnerShare;
            
            std::vector<std::string> row = {
                std::to_string(errandId),
                truncateString(desc, 25),
                truncateString(pickup, 15),
                truncateString(dropoff, 15),
                distStream.str() + " km",
                "RM " + earnStream.str()
            };
            printTableRow(row, widths);
        }

        if (!hasErrands) {
            centerText("No available errands at the moment.");
        }
        
        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
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
        
        clearScreen();
        printMenuTitle("My Assigned Errands");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Description", 25},
            {"Pickup", 15},
            {"Dropoff", 15},
            {"Distance", 10},
            {"Status", 12}
        };
        std::vector<int> widths = {5, 25, 15, 15, 10, 12};
        
        printTableHeader(columns);
        
        bool hasErrands = false;
        while (res->next()) {
            hasErrands = true;
            std::ostringstream distStream;
            distStream << std::fixed << std::setprecision(2) << res->getDouble("distance");
            
            std::vector<std::string> row = {
                std::to_string(res->getInt("errand_id")),
                truncateString(res->getString("description"), 25),
                truncateString(res->getString("pickup_loc"), 15),
                truncateString(res->getString("dropoff_loc"), 15),
                distStream.str() + " km",
                truncateString(res->getString("status"), 12)
            };
            printTableRow(row, widths);
        }
        if (!hasErrands) {
            centerText("No errands assigned to you yet.");
        }
        
        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        std::cin.get();

    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Accept an errand =====
void acceptErrand(const std::string& username) {
    int runnerId = getRunnerId(username);
    if (runnerId == -1) { std::cout << "Runner not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        // Tunjuk semua errands pending dengan runner_share
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT e.errand_id, e.description, e.pickup_loc, e.dropoff_loc, e.distance, "
                "q.runner_share, q.base_price_per_km, q.runner_percentage "
                "FROM errands e "
                "LEFT JOIN quotations q ON e.errand_id = q.errand_id "
                "WHERE e.status='Pending' "
                "ORDER BY e.created_at ASC"
            )
        );

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        
        clearScreen();
        printMenuTitle("Available Errands - Accept");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Description", 25},
            {"Pickup", 15},
            {"Dropoff", 15},
            {"Distance", 10},
            {"Runner Earn", 12}
        };
        std::vector<int> widths = {5, 25, 15, 15, 10, 12};
        
        printTableHeader(columns);
        
        bool hasErrands = false;
        // Simpan errand info dalam map untuk lookup nanti
        std::map<int, double> errandRunnerShare;

        while (res->next()) {
            hasErrands = true;
            int errandId = res->getInt("errand_id");
            std::string desc = res->getString("description");
            std::string pickup = res->getString("pickup_loc");
            std::string dropoff = res->getString("dropoff_loc");
            double distance = res->getDouble("distance");

            double runnerShare = 0.0;
            if (!res->isNull("runner_share")) {
                runnerShare = res->getDouble("runner_share");
            }
            else {
                double basePricePerKm = !res->isNull("base_price_per_km") ? res->getDouble("base_price_per_km") : 3.0;
                double runnerPercentage = !res->isNull("runner_percentage") ? res->getDouble("runner_percentage") : 70.0;
                runnerShare = distance * basePricePerKm * (runnerPercentage / 100.0);
            }

            errandRunnerShare[errandId] = runnerShare;
            
            std::ostringstream distStream, earnStream;
            distStream << std::fixed << std::setprecision(2) << distance;
            earnStream << std::fixed << std::setprecision(2) << runnerShare;
            
            std::vector<std::string> row = {
                std::to_string(errandId),
                truncateString(desc, 25),
                truncateString(pickup, 15),
                truncateString(dropoff, 15),
                distStream.str() + " km",
                "RM " + earnStream.str()
            };
            printTableRow(row, widths);
        }

        if (!hasErrands) {
            centerText("No available errands at the moment.");
            printTableFooter(widths);
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        
        printTableFooter(widths);

        int errandId;
        while (true) {
            errandId = getCenteredIntInput("Enter the ID of the errand to accept (or 0 to go back): ");
            if (errandId == 0) return; // Back to menu
            if (errandRunnerShare.find(errandId) != errandRunnerShare.end()) {
                break;
            }
            else {
                printError("Invalid input! ID not found.");
            }
        }

        std::ostringstream earnStream;
        earnStream << std::fixed << std::setprecision(2) << errandRunnerShare[errandId];
        centerText("You will earn RM " + earnStream.str() + " for this errand.");
        std::cout << "\n";

        char confirm;
        while (true) {
            std::string confirmInput = getCenteredInput("Confirm acceptance of this errand? (Y/N): ");
            if (confirmInput.length() == 1) {
                confirm = confirmInput[0];
                if (confirm == 'Y' || confirm == 'y' || confirm == 'N' || confirm == 'n') break;
            }
            printError("Invalid input! Enter Y or N.");
        }
        if (confirm == 'N' || confirm == 'n') {
            printInfo("Operation canceled.");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }

        // Update errand status
        std::unique_ptr<sql::PreparedStatement> updateStmt(
            con->prepareStatement(
                "UPDATE errands SET status='Assigned', runner_id=? "
                "WHERE errand_id=? AND status='Pending'"
            )
        );
        updateStmt->setInt(1, runnerId);
        updateStmt->setInt(2, errandId);

        int updated = updateStmt->executeUpdate();
        if (updated > 0) {
            printSuccess("Errand accepted successfully!");
        }
        else {
            printError("Failed to accept errand!");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.get();

    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Runner summary stats =====
void viewRunnerStats(const std::string& username) {
    int runnerId = getRunnerId(username);
    if (runnerId == -1) {
        std::cout << "Runner not found!\n";
        return;
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        // Ambik semua errands untuk runner ni
        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT e.status, e.distance, q.runner_share, q.base_price_per_km, q.runner_percentage "
                "FROM errands e "
                "LEFT JOIN quotations q ON e.errand_id = q.errand_id "
                "WHERE e.runner_id=?"
            )
        );
        pstmt->setInt(1, runnerId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        int total = 0, pending = 0, assigned = 0, completed = 0;
        double totalEarned = 0.0;

        while (res->next()) {
            total++;
            std::string status = res->getString("status");

            double distance = res->getDouble("distance");
            double runnerShare = 0.0;

            // Ambik runner_share dari quotations jika ada
            if (!res->isNull("runner_share")) {
                runnerShare = res->getDouble("runner_share");
            }
            else {
                double basePricePerKm = !res->isNull("base_price_per_km") ? res->getDouble("base_price_per_km") : 3.0;
                double runnerPercentage = !res->isNull("runner_percentage") ? res->getDouble("runner_percentage") : 70.0;
                runnerShare = distance * basePricePerKm * (runnerPercentage / 100.0);
            }

            if (status == "Pending") pending++;
            else if (status == "Assigned") assigned++;
            else if (status == "Completed") {
                completed++;
                totalEarned += runnerShare;
            }
        }

        clearScreen();
        printMenuTitle("Runner Summary Statistics");
        
        std::ostringstream earnStream;
        earnStream << std::fixed << std::setprecision(2) << totalEarned;
        
        centerText("+------------------------------------------+");
        centerText("|        YOUR RUNNER STATISTICS            |");
        centerText("+------------------------------------------+");
        centerText("| Total Errands    : " + std::to_string(total) + std::string(20, ' ') + "|");
        centerText("| Pending          : " + std::to_string(pending) + std::string(20, ' ') + "|");
        centerText("| Assigned         : " + std::to_string(assigned) + std::string(20, ' ') + "|");
        centerText("| Completed        : " + std::to_string(completed) + std::string(20, ' ') + "|");
        centerText("| Total Earned     : RM " + earnStream.str() + std::string(15, ' ') + "|");
        centerText("+------------------------------------------+");
        std::cout << "\nPress Enter to continue...";
        std::cin.get();

    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}

// ===== Claim earned errands =====
void claimErrandEarnings(const std::string& username) {
    int runnerId = getRunnerId(username);
    if (runnerId == -1) { std::cout << "Runner not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT e.errand_id, e.description, "
                "COALESCE(q.runner_share, e.distance * 3 * 0.7) AS runner_share "
                "FROM errands e "
                "LEFT JOIN quotations q ON e.errand_id = q.errand_id "
                "WHERE e.runner_id=? AND e.status='Completed' AND (e.runner_earned=FALSE OR e.runner_earned IS NULL)"
            )
        );
        pstmt->setInt(1, runnerId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        
        clearScreen();
        printMenuTitle("Claim Errand Earnings");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"Errand ID", 10},
            {"Description", 40},
            {"Earnings", 15}
        };
        std::vector<int> widths = {10, 40, 15};
        
        printTableHeader(columns);
        
        bool hasEarnings = false;
        double totalEarned = 0;

        while (res->next()) {
            hasEarnings = true;
            int errandId = res->getInt("errand_id");
            std::string desc = res->getString("description");
            double runnerShare = res->getDouble("runner_share");

            std::ostringstream earnStream;
            earnStream << std::fixed << std::setprecision(2) << runnerShare;
            
            std::vector<std::string> row = {
                std::to_string(errandId),
                truncateString(desc, 40),
                "RM " + earnStream.str()
            };
            printTableRow(row, widths);

            totalEarned += runnerShare;

            // Mark as earned
            std::unique_ptr<sql::PreparedStatement> updateStmt(
                con->prepareStatement("UPDATE errands SET runner_earned=TRUE WHERE errand_id=?")
            );
            updateStmt->setInt(1, errandId);
            updateStmt->executeUpdate();
        }

        if (!hasEarnings) {
            centerText("No earnings to claim yet.");
        }
        else {
            std::ostringstream totalStream;
            totalStream << std::fixed << std::setprecision(2) << totalEarned;
            centerText("Total earned this session: RM " + totalStream.str());
        }
        
        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Full runner menu =====
void runner_menu(const std::string& username) {
    while (true) {
        clearScreen();
        printMenuTitle("Runner Dashboard - Welcome " + username);
        printHeader("RUNNER MENU");
        centerText("1. View available errands");
        centerText("2. Accept an errand");
        centerText("3. View my assigned errands");
        centerText("4. View summary stats");
        centerText("5. Claim earned errands");
        centerText("0. Logout");
        printHeader("");
        std::cout << "\n";
        centerText("Enter choice: ");
        int choice = getMenuChoice(0, 6);

        switch (choice) {
        case 0: return;
        case 1: viewAvailableErrands(); break;
        case 2: acceptErrand(username); break;
        case 3: viewAssignedErrands(username); break;
        case 4: viewRunnerStats(username); break;
        case 5: claimErrandEarnings(username); break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}

