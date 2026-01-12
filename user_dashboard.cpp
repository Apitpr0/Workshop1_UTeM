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
#include <sstream>
#include <cctype>   // for std::isdigit
#include <cmath>    // for std::isnan, std::isinf
#include "utils.h"  // untuk getMenuChoice dan UI functions

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

    clearScreen();
    printMenuTitle("View My Errands");
    printHeader("FILTER ERRANDS BY STATUS");
    centerText("1. All");
    centerText("2. Pending");
    centerText("3. Assigned");
    centerText("4. Completed");
    centerText("0. Back");
    printHeader("");
    std::cout << "\n";
    
    int filterChoice = 0;
    while (true) {
        filterChoice = getCenteredIntInput("Enter filter choice (0-4): ");
        if (filterChoice >= 0 && filterChoice <= 4) {
            if (filterChoice == 0) return; // Back to menu
            break;
        }
        else {
            printError("Invalid choice! Enter a number between 0 and 4.");
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
        
        clearScreen();
        printMenuTitle("Your Errands");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Description", 25},
            {"Pickup", 15},
            {"Dropoff", 15},
            {"Distance", 10},
            {"Status", 12},
            {"Runner", 15},
            {"Created", 20}
        };
        std::vector<int> widths = {5, 25, 15, 15, 10, 12, 15, 20};
        
        printTableHeader(columns);
        
        bool hasErrands = false;
        while (res->next()) {
            hasErrands = true;
            std::string runnerName = "N/A";
            if (res->getString("status") == "Assigned") {
                runnerName = res->getString("runner_name");
                if (runnerName.empty()) runnerName = "N/A";
            }
            std::ostringstream distStream;
            distStream << std::fixed << std::setprecision(2) << res->getDouble("distance");
            
            std::vector<std::string> row = {
                std::to_string(res->getInt("errand_id")),
                truncateString(res->getString("description"), 25),
                truncateString(res->getString("pickup_loc"), 15),
                truncateString(res->getString("dropoff_loc"), 15),
                distStream.str() + " km",
                truncateString(res->getString("status"), 12),
                truncateString(runnerName, 15),
                truncateString(res->getString("created_at"), 20)
            };
            printTableRow(row, widths);
        }
        if (!hasErrands) {
            centerText("No errands found!");
        }
        
        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== View quotations =====
void viewQuotation(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) {
        std::cout << "User not found!\n";
        return;
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(
            driver->connect("tcp://localhost:3306", "root", "")
        );
        con->setSchema("erms");

        std::unique_ptr<sql::PreparedStatement> pstmt(
            con->prepareStatement(
                "SELECT q.quote_id, q.errand_id, q.base_price_per_km, q.distance_km, "
                "q.runner_percentage, q.system_percentage, q.runner_share, q.system_fee, q.status "
                "FROM quotations q "
                "JOIN errands e ON q.errand_id = e.errand_id "
                "WHERE e.requester_id=? AND q.status='Pending' "
                "ORDER BY q.quote_id DESC"
            )
        );
        pstmt->setInt(1, userId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        clearScreen();
        printMenuTitle("Pending Quotations");

        bool hasPending = false;
        while (res->next()) {
            hasPending = true;
            std::ostringstream baseStream, runnerFeeStream, systemFeeStream;
            baseStream << std::fixed << std::setprecision(2) << res->getDouble("base_price_per_km");
            runnerFeeStream << std::fixed << std::setprecision(2) << res->getDouble("runner_share");
            systemFeeStream << std::fixed << std::setprecision(2) << res->getDouble("system_fee");
            
            centerText("+------------------------------------------+");
            centerText("|        QUOTATION INFORMATION              |");
            centerText("+------------------------------------------+");
            centerText("| Quotation ID   : " + std::to_string(res->getInt("quote_id")) + std::string(20, ' ') + "|");
            centerText("| Errand ID      : " + std::to_string(res->getInt("errand_id")) + std::string(20, ' ') + "|");
            centerText("| Base Price/km  : RM " + baseStream.str() + std::string(15, ' ') + "|");
            centerText("| Distance       : " + std::to_string(static_cast<int>(res->getDouble("distance_km"))) + " km" + std::string(18, ' ') + "|");
            centerText("| Runner %       : " + std::to_string(static_cast<int>(res->getDouble("runner_percentage"))) + "%" + std::string(19, ' ') + "|");
            centerText("| System %       : " + std::to_string(static_cast<int>(res->getDouble("system_percentage"))) + "%" + std::string(19, ' ') + "|");
            centerText("| Runner Fee     : RM " + runnerFeeStream.str() + std::string(15, ' ') + "|");
            centerText("| System Fee     : RM " + systemFeeStream.str() + std::string(15, ' ') + "|");
            centerText("| Status         : " + res->getString("status") + std::string(20, ' ') + "|");
            centerText("+------------------------------------------+");
            std::cout << "\n";
        }

        if (!hasPending) {
            printInfo("All quotations have been paid.");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.get();

    }
    catch (sql::SQLException& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}


void makePayment(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) {
        printError("User not found! Cannot proceed with payment.");
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        return;
    }

    clearScreen();
    printMenuTitle("Make Payment");
	viewQuotation(username);
    
    int quoteId;
    quoteId = getCenteredIntInput("Enter Quotation ID to pay (or 0 to go back): ");
    if (quoteId == 0) return; // Back to menu
    
    if (quoteId < 1) {
        printError("Invalid quotation ID! Please enter a valid ID.");
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        return;
    }

    // Validate quotation exists and belongs to user before asking for payment details
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");

        // Check if quotation exists, is pending, and belongs to this user
        std::unique_ptr<sql::PreparedStatement> checkStmt(
            con->prepareStatement(
                "SELECT q.quote_id, q.status "
                "FROM quotations q "
                "JOIN errands e ON q.errand_id = e.errand_id "
                "WHERE q.quote_id=? AND e.requester_id=?"
            )
        );
        checkStmt->setInt(1, quoteId);
        checkStmt->setInt(2, userId);

        std::unique_ptr<sql::ResultSet> checkRes(checkStmt->executeQuery());
        if (!checkRes->next()) {
            printError("Quotation not found or does not belong to you!");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        
        std::string quoteStatus = checkRes->getString("status");
        if (quoteStatus != "Pending") {
            printError("This quotation has already been paid or is not available for payment!");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
    }
    catch (sql::SQLException& e) {
        printError("Database connection error! Please try again later.");
        std::cerr << "Database error: " << e.what() << std::endl;
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        return;
    }
    catch (std::exception& e) {
        printError("An unexpected error occurred while validating quotation!");
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        return;
    }

    clearScreen();
    printMenuTitle("Payment Method: Credit / Debit Card");
    printInfo("Type 'back' at any time to cancel");
    std::cout << "\n";
    
    std::string cardNumber, expiryDate, cvv;
    
    // Validate card number
    while (true) {
        cardNumber = getCenteredInput("Card Number (12-19 digits, or 'back' to cancel): ");
        if (cardNumber == "back" || cardNumber == "Back" || cardNumber == "BACK") {
            printInfo("Payment cancelled.");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        
        // Remove spaces and dashes for validation
        std::string cardNumberClean;
        for (char c : cardNumber) {
            if (std::isdigit(c)) cardNumberClean += c;
        }
        
        if (cardNumberClean.empty()) {
            printError("Card number cannot be empty! Please enter valid digits.");
            continue;
        }
        
        if (cardNumberClean.length() < 12 || cardNumberClean.length() > 19) {
            printError("Card number must be between 12 and 19 digits!");
            continue;
        }
        
        // Check if all characters are digits (after cleaning)
        bool allDigits = true;
        for (char c : cardNumberClean) {
            if (!std::isdigit(c)) {
                allDigits = false;
                break;
            }
        }
        
        if (!allDigits) {
            printError("Card number must contain only digits!");
            continue;
        }
        
        cardNumber = cardNumberClean;
        break;
    }

    // Validate expiry date (MM/YY format)
    while (true) {
        expiryDate = getCenteredInput("Expiry Date (MM/YY, or 'back' to cancel): ");
        if (expiryDate == "back" || expiryDate == "Back" || expiryDate == "BACK") {
            printInfo("Payment cancelled.");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        
        if (expiryDate.length() != 5 || expiryDate[2] != '/') {
            printError("Invalid format! Please use MM/YY format (e.g., 12/25)");
            continue;
        }
        
        std::string monthStr = expiryDate.substr(0, 2);
        std::string yearStr = expiryDate.substr(3, 2);
        
        // Check if month and year are numeric
        bool validMonth = true, validYear = true;
        for (char c : monthStr) {
            if (!std::isdigit(c)) {
                validMonth = false;
                break;
            }
        }
        for (char c : yearStr) {
            if (!std::isdigit(c)) {
                validYear = false;
                break;
            }
        }
        
        if (!validMonth || !validYear) {
            printError("Month and year must be numeric! Use MM/YY format.");
            continue;
        }
        
        int month = std::stoi(monthStr);
        int year = std::stoi(yearStr);
        
        if (month < 1 || month > 12) {
            printError("Invalid month! Month must be between 01 and 12.");
            continue;
        }
        
        // Basic expiry validation (should be future date, but simplified for now)
        if (year < 0 || year > 99) {
            printError("Invalid year! Year must be between 00 and 99.");
            continue;
        }
        
        break;
    }

    // Validate CVV
    while (true) {
        cvv = getCenteredInput("CVV (3 digits, or 'back' to cancel): ");
        if (cvv == "back" || cvv == "Back" || cvv == "BACK") {
            printInfo("Payment cancelled.");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        
        if (cvv.length() != 3) {
            printError("CVV must be exactly 3 digits!");
            continue;
        }
        
        bool allDigits = true;
        for (char c : cvv) {
            if (!std::isdigit(c)) {
                allDigits = false;
                break;
            }
        }
        
        if (!allDigits) {
            printError("CVV must contain only digits!");
            continue;
        }
        
        break;
    }

    // Process payment with transaction safety
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
        con->setSchema("erms");
        
        // Disable autocommit for transaction safety
        con->setAutoCommit(false);

        // Get quotation details and verify again (double-check)
        std::unique_ptr<sql::PreparedStatement> qStmt(
            con->prepareStatement(
                "SELECT q.errand_id, q.base_price_per_km, q.distance_km, "
                "q.runner_percentage, q.system_percentage, q.status, e.requester_id "
                "FROM quotations q "
                "JOIN errands e ON q.errand_id = e.errand_id "
                "WHERE q.quote_id=? AND e.requester_id=? AND q.status='Pending'"
            )
        );
        qStmt->setInt(1, quoteId);
        qStmt->setInt(2, userId);

        std::unique_ptr<sql::ResultSet> qRes(qStmt->executeQuery());
        if (!qRes->next()) {
            con->rollback();
            printError("Quotation not found, already paid, or does not belong to you!");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }

        int errandId = qRes->getInt("errand_id");
        double base = qRes->getDouble("base_price_per_km");
        double distance = qRes->getDouble("distance_km");
        double runnerPerc = qRes->getDouble("runner_percentage");
        double systemPerc = qRes->getDouble("system_percentage");

        // Validate calculated values
        if (base <= 0 || distance <= 0 || runnerPerc < 0 || systemPerc < 0) {
            con->rollback();
            printError("Invalid quotation data! Please contact support.");
            std::cerr << "Error: Invalid quotation values detected." << std::endl;
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }

        double total = base * distance;
        double runnerFee = total * runnerPerc / 100.0;
        double systemFee = total * systemPerc / 100.0;

        // Validate total amount
        if (total <= 0 || std::isnan(total) || std::isinf(total)) {
            con->rollback();
            printError("Invalid payment amount calculated! Please contact support.");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }

        // Generate transaction ID
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        std::string transactionId = "ERMS#" + std::to_string(100000 + std::rand() % 900000);

        std::string payStatus = "Paid";

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
        int updatedRows = uStmt->executeUpdate();
        
        if (updatedRows == 0) {
            con->rollback();
            printError("Failed to update quotation! Payment not processed.");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }

        // Commit transaction
        con->commit();
        con->setAutoCommit(true);

        // ================= RECEIPT =================
        clearScreen();
        printMenuTitle("Payment Successful!");
        
        std::ostringstream baseStream, totalStream, runnerFeeStream, systemFeeStream;
        baseStream << std::fixed << std::setprecision(2) << base;
        totalStream << std::fixed << std::setprecision(2) << total;
        runnerFeeStream << std::fixed << std::setprecision(2) << runnerFee;
        systemFeeStream << std::fixed << std::setprecision(2) << systemFee;
        
        centerText("==========================================");
        centerText("       ERMS PAYMENT RECEIPT              ");
        centerText("==========================================");
        centerText("Transaction ID : " + transactionId);
        centerText("Quotation ID   : " + std::to_string(quoteId));
        centerText("Errand ID      : " + std::to_string(errandId));
        centerText("------------------------------------------");
        centerText("Base Price/km  : RM " + baseStream.str());
        centerText("Distance       : " + std::to_string(static_cast<int>(distance)) + " km");
        centerText("------------------------------------------");
        centerText("Total Paid     : RM " + totalStream.str());
        centerText("Runner (" + std::to_string(static_cast<int>(runnerPerc)) + "%)   : RM " + runnerFeeStream.str());
        centerText("System (" + std::to_string(static_cast<int>(systemPerc)) + "%)   : RM " + systemFeeStream.str());
        centerText("------------------------------------------");
        centerText("Payment Status : " + payStatus);
        centerText("Method         : Credit / Debit Card");
        centerText("==========================================");
        std::cout << "\nPress Enter to continue...";
        std::cin.get();

    }
    catch (sql::SQLException& e) {
        printError("Database error occurred during payment processing!");
        std::cerr << "SQL Error Code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQL State: " << e.getSQLState() << std::endl;
        std::cerr << "Error Message: " << e.what() << std::endl;
        
        // Try to rollback if connection is still valid
        try {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            std::unique_ptr<sql::Connection> con(driver->connect("tcp://localhost:3306", "root", ""));
            con->setSchema("erms");
            con->rollback();
        }
        catch (...) {
            // Ignore rollback errors
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    catch (std::exception& e) {
        printError("An unexpected error occurred during payment processing!");
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    catch (...) {
        printError("A critical error occurred! Payment may not have been processed.");
        std::cerr << "Unknown error occurred during payment processing." << std::endl;
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
}

// ===== Create new errand (with auto quotation) =====
void createNewErrand(const std::string& username) {
    int userId = getUserId(username);
    if (userId == -1) { std::cout << "User not found!\n"; return; }

    std::string desc, pickup, dropoff;
    do {
        desc = getCenteredInput("Enter errand description (or 'back' to cancel): ");
        if (desc == "back" || desc == "Back" || desc == "BACK") {
            centerText("Errand creation cancelled.");
            return;
        }
        if (desc.empty()) centerText("Description cannot be empty!");
    } while (desc.empty());

    do {
        pickup = getCenteredInput("Enter pickup location (or 'back' to cancel): ");
        if (pickup == "back" || pickup == "Back" || pickup == "BACK") {
            centerText("Errand creation cancelled.");
            return;
        }
        if (pickup.empty()) centerText("Pickup location cannot be empty!");
    } while (pickup.empty());

    do {
        dropoff = getCenteredInput("Enter dropoff location (or 'back' to cancel): ");
        if (dropoff == "back" || dropoff == "Back" || dropoff == "BACK") {
            centerText("Errand creation cancelled.");
            return;
        }
        if (dropoff.empty()) centerText("Dropoff location cannot be empty!");
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

        std::ostringstream distStream;
        distStream << std::fixed << std::setprecision(2) << distance;
        printSuccess("New errand added successfully! Distance: " + distStream.str() + " km");

        // Post-create option
        std::cout << "\n";
        printHeader("WHAT WOULD YOU LIKE TO DO?");
        centerText("1. View quotations");
        centerText("2. Back to dashboard");
        printHeader("");
        std::cout << "\n";
        
        int postChoice = 0;
        while (true) {
            postChoice = getCenteredIntInput("Enter choice (1 or 2): ");
            if (postChoice == 1 || postChoice == 2) {
                break;
            }
            else {
                printError("Invalid choice! Enter 1 or 2.");
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
                "WHERE requester_id=? AND (status='Assigned') ORDER BY created_at DESC"
            )
        );
        pstmt->setInt(1, userId);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        
        clearScreen();
        printMenuTitle("Mark Errand as Completed");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Description", 50},
            {"Status", 12}
        };
        std::vector<int> widths = {5, 50, 12};
        
        printTableHeader(columns);
        
        bool hasErrands = false;
        while (res->next()) {
            hasErrands = true;
            std::vector<std::string> row = {
                std::to_string(res->getInt("errand_id")),
                truncateString(res->getString("description"), 50),
                truncateString(res->getString("status"), 12)
            };
            printTableRow(row, widths);
        }
        if (!hasErrands) {
            centerText("No pending or assigned errands.");
            printTableFooter(widths);
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        printTableFooter(widths);

        int errandId;
        while (true) {
            errandId = getCenteredIntInput("Enter the ID of the errand to mark as COMPLETED (or 0 to go back): ");
            if (errandId == 0) return; // Back to menu
            break;
        }

        char confirm;
        while (true) {
            std::string confirmInput = getCenteredInput("Are you sure you want to mark this errand as COMPLETED? (Y/N): ");
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

        std::unique_ptr<sql::PreparedStatement> updateStmt(
            con->prepareStatement("UPDATE errands SET status='Completed' WHERE requester_id=? AND errand_id=?")
        );
        updateStmt->setInt(1, userId);
        updateStmt->setInt(2, errandId);

        int updated = updateStmt->executeUpdate();
        if (updated > 0) {
            printSuccess("Errand marked as COMPLETED successfully!");
        }
        else {
            printError("Errand not found or cannot be marked as completed!");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.get();

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
        
        clearScreen();
        printMenuTitle("Cancel Pending Errands");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Description", 50}
        };
        std::vector<int> widths = {5, 50};
        
        printTableHeader(columns);
        
        while (res->next()) {
            hasPending = true;
            std::vector<std::string> row = {
                std::to_string(res->getInt("errand_id")),
                truncateString(res->getString("description"), 50)
            };
            printTableRow(row, widths);
        }
        if (!hasPending) {
            centerText("No pending errands.");
            printTableFooter(widths);
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        printTableFooter(widths);
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; return; }

    int errandId;
    while (true) {
        errandId = getCenteredIntInput("Enter the ID of the errand to cancel (or 0 to go back): ");
        if (errandId == 0) return; // Back to menu
        break;
    }

    char confirm;
    while (true) {
        std::string confirmInput = getCenteredInput("Are you sure you want to CANCEL this errand? (Y/N): ");
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
        if (deleted > 0) {
            printSuccess("Errand canceled successfully!");
        }
        else {
            printError("Errand not found, not pending, or not linked to you!");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
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
            clearScreen();
            printMenuTitle("Errand Summary Statistics");
            
            centerText("+------------------------------------------+");
            centerText("|        YOUR ERRAND STATISTICS           |");
            centerText("+------------------------------------------+");
            centerText("| Total Errands    : " + std::to_string(res->getInt("total")) + std::string(20, ' ') + "|");
            centerText("| Pending          : " + std::to_string(res->getInt("pending")) + std::string(20, ' ') + "|");
            centerText("| Assigned         : " + std::to_string(res->getInt("assigned")) + std::string(20, ' ') + "|");
            centerText("| Completed        : " + std::to_string(res->getInt("completed")) + std::string(20, ' ') + "|");
            centerText("+------------------------------------------+");
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
        }
    }
    catch (sql::SQLException& e) { std::cerr << "Database error: " << e.what() << std::endl; }
}

// ===== Full user menu =====
void user_menu(const std::string& username) {
    while (true) {
        clearScreen();
        printMenuTitle("User Dashboard - Welcome " + username);
        printHeader("USER MENU");
        centerText("1. View my errands (with filter)");
        centerText("2. Create new errand");
        centerText("3. Mark an errand as completed");
        centerText("4. Cancel pending errand");
        centerText("5. View summary stats");
        centerText("6. View quotations");
        centerText("7. Make payment");
        centerText("0. Logout");
        printHeader("");
        std::cout << "\n";
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
