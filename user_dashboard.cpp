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

// ===== Create quotation automatically =====
void createQuotation(int errandId, double distance) {
    try {
        double basePricePerKm = 3.0; // RM3 per km
        double runnerPercentage = 70.0;
        double systemPercentage = 30.0;

        double runnerFee = distance * basePricePerKm * (runnerPercentage / 100.0);
        double systemFee = distance * basePricePerKm * (systemPercentage / 100.0);

        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "INSERT INTO quotations(errand_id, base_price_per_km, distance_km, runner_percentage, system_percentage, runner_share, system_fee, status) "
                "VALUES(?, ?, ?, ?, ?, ?, ?, 'Pending')"
            )
        );
        pstmt->setInt(1, errandId);
        pstmt->setDouble(2, basePricePerKm);
        pstmt->setDouble(3, distance);
        pstmt->setDouble(4, runnerPercentage);
        pstmt->setDouble(5, systemPercentage);
        pstmt->setDouble(6, runnerFee);
        pstmt->setDouble(7, systemFee);
        pstmt->execute();
    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error (createQuotation): " << e.what() << std::endl;
    }
}

// ===== View my errands =====
void viewMyErrands(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    int filterChoice = 0;
    while (true) {
        std::cout << "\nFilter errands by status:\n";
        std::cout << "1. All\n2. Pending\n3. Assigned\n4. Completed\nEnter choice: ";
        if (std::cin >> filterChoice && filterChoice >= 1 && filterChoice <= 4) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid choice! Enter a number between 1 and 4.\n";
        }
    }

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
                "SELECT e.errand_id, e.description, e.pickup_loc, e.dropoff_loc, e.distance, e.status, e.created_at, "
                "u.name AS runner_name "
                "FROM errands e LEFT JOIN users u ON e.runner_id = u.user_id "
                "WHERE e.requester_id=? ORDER BY e.created_at DESC"
            ));
        }
        else {
            pstmt.reset(con->prepareStatement(
                "SELECT e.errand_id, e.description, e.pickup_loc, e.dropoff_loc, e.distance, e.status, e.created_at, "
                "u.name AS runner_name "
                "FROM errands e LEFT JOIN users u ON e.runner_id = u.user_id "
                "WHERE e.requester_id=? AND e.status=? ORDER BY e.created_at DESC"
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
                << " | Status: " << res->getString("status");
            if (res->getString("status") == "Assigned") {
                std::string runnerName = res->getString("runner_name");
                if (runnerName.empty()) runnerName = "N/A";
                std::cout << " | Runner: " << runnerName;
            }
            std::cout << " | Created: " << res->getString("created_at") << "\n";
        }
        if (!hasErrands) std::cout << "No errands found!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== View quotations =====
void viewQuotation(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT q.quote_id, q.errand_id, q.base_price_per_km, q.distance_km, "
                "q.runner_percentage, q.system_percentage, q.runner_share, q.system_fee, q.status, q.transaction_id "
                "FROM quotations q JOIN errands e ON q.errand_id = e.errand_id "
                "WHERE e.requester_id=? ORDER BY q.quote_id DESC"
            )
        );
        pstmt->setInt(1, userId);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        std::cout << "\n=== Your Quotations ===\n";
        bool hasQuote = false;
        while (res->next()) {
            hasQuote = true;
            std::cout << "--------------------------------------------\n";
            std::cout << "Quotation ID   : " << res->getInt("quote_id") << "\n";
            std::cout << "Errand ID      : " << res->getInt("errand_id") << "\n";
            std::cout << "Base Price/km  : RM " << std::fixed << std::setprecision(2) << res->getDouble("base_price_per_km") << "\n";
            std::cout << "Distance       : " << res->getDouble("distance_km") << " km\n";
            std::cout << "Runner %       : " << res->getDouble("runner_percentage") << "%\n";
            std::cout << "System %       : " << res->getDouble("system_percentage") << "%\n";
            std::cout << "Runner Fee     : RM " << res->getDouble("runner_share") << "\n";
            std::cout << "System Fee     : RM " << res->getDouble("system_fee") << "\n";
            std::cout << "Status         : " << res->getString("status") << "\n";
            std::cout << "Transaction ID : " << (res->isNull("transaction_id") ? "N/A" : res->getString("transaction_id")) << "\n";
            std::cout << "--------------------------------------------\n\n";
        }
        if (!hasQuote) std::cout << "No quotations found!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

void makePayment(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) {
        std::cout << "User not found!\n";
        return;
    }

    int quoteId;
    std::cout << "Enter Quotation ID to pay: ";
    std::cin >> quoteId;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string cardNumber, expiryDate, cvv;
    std::cout << "\n--- Payment Method: Credit / Debit Card ---\n";
    std::cout << "Card Number (16 digits): ";
    std::getline(std::cin, cardNumber);

    std::cout << "Expiry Date (MM/YY): ";
    std::getline(std::cin, expiryDate);

    std::cout << "CVV (3 digits): ";
    std::getline(std::cin, cvv);

    if (cardNumber.length() < 12 || cvv.length() < 3) {
        std::cout << "Invalid card details!\n";
        return;
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        // Get quotation details
        std::unique_ptr<sql::PreparedStatement> qStmt(
            con->prepareStatement(
                "SELECT errand_id, base_price_per_km, distance_km, "
                "runner_percentage, system_percentage "
                "FROM quotations WHERE quote_id=? AND status='Pending'"
            )
        );
        qStmt->setInt(1, quoteId);

        std::unique_ptr<sql::ResultSet> qRes(qStmt->executeQuery());
        if (!qRes->next()) {
            std::cout << "Quotation not found or already paid!\n";
            return;
        }

        int errandId = qRes->getInt("errand_id");
        double base = qRes->getDouble("base_price_per_km");
        double distance = qRes->getDouble("distance_km");
        double runnerPerc = qRes->getDouble("runner_percentage");
        double systemPerc = qRes->getDouble("system_percentage");

        double total = base * distance;
        double runnerFee = total * runnerPerc / 100.0;
        double systemFee = total * systemPerc / 100.0;

        // Generate transaction ID
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        std::string transactionId = "ERMS#" + std::to_string(100000 + std::rand() % 900000);

        std::string payStatus = "Paid"; // gunakan variable, boleh extend nanti

        // Insert payment
        std::unique_ptr<sql::PreparedStatement> pStmt(
            con->prepareStatement(
                "INSERT INTO payments(quote_id, errand_id, price, pay_status, transaction_id) "
                "VALUES(?, ?, ?, ?, ?)"
            )
        );
        pStmt->setInt(1, quoteId);
        pStmt->setInt(2, errandId);
        pStmt->setDouble(3, total);
        pStmt->setString(4, payStatus);
        pStmt->setString(5, transactionId);
        pStmt->execute();

        // Update quotation
        std::unique_ptr<sql::PreparedStatement> uStmt(
            con->prepareStatement(
                "UPDATE quotations SET status='Paid', "
                "runner_share=?, system_fee=?, transaction_id=? "
                "WHERE quote_id=?"
            )
        );
        uStmt->setDouble(1, runnerFee);
        uStmt->setDouble(2, systemFee);
        uStmt->setString(3, transactionId);
        uStmt->setInt(4, quoteId);
        uStmt->executeUpdate();

        // ================= RECEIPT =================
        std::cout << "\n=====================================\n";
        std::cout << "          ERMS PAYMENT RECEIPT        \n";
        std::cout << "=====================================\n";
        std::cout << "Transaction ID : " << transactionId << "\n";
        std::cout << "Quotation ID   : " << quoteId << "\n";
        std::cout << "Errand ID      : " << errandId << "\n";
        std::cout << "-------------------------------------\n";
        std::cout << "Base Price/km  : RM " << std::fixed << std::setprecision(2) << base << "\n";
        std::cout << "Distance       : " << distance << " km\n";
        std::cout << "-------------------------------------\n";
        std::cout << "Total Paid     : RM " << total << "\n";
        std::cout << "Runner (" << runnerPerc << "%)   : RM " << runnerFee << "\n";
        std::cout << "System (" << systemPerc << "%)   : RM " << systemFee << "\n";
        std::cout << "-------------------------------------\n";
        std::cout << "Payment Status : " << payStatus << "\n";
        std::cout << "Method         : Credit / Debit Card\n";
        std::cout << "=====================================\n";

    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}

// ===== Create new errand (with auto quotation) =====
void createNewErrand(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    std::string desc, pickup, dropoff;
    do {
        std::cout << "Enter errand description: "; std::getline(std::cin, desc);
        if (desc.empty()) std::cout << "Description cannot be empty!\n";
    } while (desc.empty());

    do {
        std::cout << "Enter pickup location: "; std::getline(std::cin, pickup);
        if (pickup.empty()) std::cout << "Pickup location cannot be empty!\n";
    } while (pickup.empty());

    do {
        std::cout << "Enter dropoff location: "; std::getline(std::cin, dropoff);
        if (dropoff.empty()) std::cout << "Dropoff location cannot be empty!\n";
    } while (dropoff.empty());

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

        // Get last inserted errand ID
        std::unique_ptr<sql::PreparedStatement> lastIdStmt(
            con->prepareStatement("SELECT LAST_INSERT_ID() AS last_id")
        );
        std::unique_ptr<sql::ResultSet> res(lastIdStmt->executeQuery());
        int lastErrandId = -1;
        if (res->next()) lastErrandId = res->getInt("last_id");

        if (lastErrandId != -1) {
            createQuotation(lastErrandId, distance);
        }

        std::cout << "New errand added successfully! Distance: "
            << std::fixed << std::setprecision(2) << distance << " km\n";

        // Post-create option
        int postChoice = 0;
        while (true) {
            std::cout << "\nWhat would you like to do next?\n";
            std::cout << "1. View quotations\n2. Back to dashboard\nEnter choice: ";
            if (std::cin >> postChoice && (postChoice == 1 || postChoice == 2)) {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                break;
            }
            else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid choice! Enter 1 or 2.\n";
            }
        }
        if (postChoice == 1) viewQuotation(username);
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Update errand status, cancel, view summary as before =====
// (Reuse existing functions: updateErrandStatus, cancelPendingErrand, viewSummaryStats)
// ===== Update errand status =====
void updateErrandStatus(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT errand_id, description, status FROM errands "
                "WHERE requester_id=? AND (status='Pending' OR status='Assigned') ORDER BY created_at DESC"
            )
        );
        pstmt->setInt(1, userId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        bool hasErrands = false;
        std::cout << "\n--- Pending/Assigned Errands ---\n";
        while (res->next()) {
            hasErrands = true;
            std::cout << "ID: " << res->getInt("errand_id")
                << " | Desc: " << res->getString("description")
                << " | Status: " << res->getString("status") << "\n";
        }
        if (!hasErrands) { std::cout << "No pending or assigned errands.\n"; return; }

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
            con->prepareStatement("UPDATE errands SET status='Completed' WHERE requester_id=? AND errand_id=?")
        );
        updateStmt->setInt(1, userId);
        updateStmt->setInt(2, errandId);

        int updated = updateStmt->executeUpdate();
        if (updated > 0) std::cout << "Errand marked as COMPLETED successfully!\n";
        else std::cout << "Errand not found, not pending/assigned, or not linked to you!\n";

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
            con->prepareStatement("SELECT errand_id, description FROM errands WHERE requester_id=? AND status='Pending'")
        );
        pstmt->setInt(1, userId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        bool hasPending = false;
        std::cout << "\n--- Pending Errands ---\n";
        while (res->next()) {
            hasPending = true;
            std::cout << "ID: " << res->getInt("errand_id") << " | Desc: " << res->getString("description") << "\n";
        }
        if (!hasPending) { std::cout << "No pending errands.\n"; return; }
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; return; }

    int errandId;
    while (true) {
        std::cout << "Enter the ID of the errand to cancel: ";
        if (std::cin >> errandId) { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); break; }
        else { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); std::cout << "Invalid input!\n"; }
    }

    char confirm;
    while (true) {
        std::cout << "Are you sure you want to CANCEL this errand? (Y/N): ";
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
            con->prepareStatement("DELETE FROM errands WHERE requester_id=? AND errand_id=? AND status='Pending'")
        );
        pstmt->setInt(1, userId);
        pstmt->setInt(2, errandId);

        int deleted = pstmt->executeUpdate();
        if (deleted > 0) std::cout << "Errand canceled successfully!\n";
        else std::cout << "Errand not found, not pending, or not linked to you!\n";
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== View summary stats =====
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

// ===== Full user menu =====
void user_menu(const std::string& username) {
    while (true) {
        std::cout << "\n=== User Dashboard ===\n";
        std::cout << "1. View my errands (with filter)\n";
        std::cout << "2. Create new errand\n";
        std::cout << "3. Mark an errand as completed\n";
        std::cout << "4. Cancel pending errand\n";
        std::cout << "5. View summary stats\n";
        std::cout << "6. View quotations\n";
        std::cout << "7. Make payment\n";
        std::cout << "0. Logout\nEnter choice: ";
        int choice = getMenuChoice(0, 7);

        switch (choice) {
        case 0: return;
        case 1: viewMyErrands(username); break;
        case 2: createNewErrand(username); break;
        case 3: updateErrandStatus(username); break;
        case 4: cancelPendingErrand(username); break;
        case 5: viewSummaryStats(username); break;
        case 6: viewQuotation(username); break;
        case 7: makePayment(username); break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}
