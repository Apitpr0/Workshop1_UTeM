#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
#include <regex>
#include <vector>
#include <tuple>
#include <sstream>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include "hash.h" // untuk hashPassword
#include "utils.h" // untuk validation functions

const std::string RUNNER_ENROLLMENT_PASSWORD = "runner123";

// Helper untuk display star chart dengan kurungan (1 star = RM50)
void displayStarChart(const std::string& label, double amount) {
    int stars = static_cast<int>(amount / 50.0 + 0.5); // Round to nearest integer
    std::string starString(stars, '*');
    std::ostringstream amountStream;
    amountStream << std::fixed << std::setprecision(2) << amount;
    centerText(label + " : RM " + amountStream.str() + " (" + starString + ")");
}

// Helper untuk format transaction ID dengan prefix ERMS#
std::string formatTransactionId(const std::string& transactionId) {
    if (transactionId.empty()) {
        return "ERMS#UNKNOWN";
    }
    // Check if already has ERMS# prefix
    if (transactionId.find("ERMS#") == 0) {
        return transactionId;
    }
    // If not, add ERMS# prefix
    return "ERMS#" + transactionId;
}

// ===== View all users =====
void viewAllUsers() {
    clearScreen();
    printMenuTitle("All Users");
    
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

    sql::PreparedStatement* pstmt = con->prepareStatement(
        "SELECT user_id, name, email, c_number, role FROM users"
    );
    sql::ResultSet* res = pstmt->executeQuery();

    std::vector<std::pair<std::string, int>> columns = {
        {"ID", 5},
        {"Name", 20},
        {"Email", 25},
        {"Contact", 15},
        {"Role", 10}
    };
    std::vector<int> widths = {5, 20, 25, 15, 10};
    
    printTableHeader(columns);

    bool hasData = false;
    while (res->next()) {
        hasData = true;
        int role = res->getInt("role");
        std::string roleText = (role == 0) ? "User" : (role == 1) ? "Admin" : "Runner";

        std::vector<std::string> row = {
            std::to_string(res->getInt("user_id")),
            truncateString(res->getString("name"), 20),
            truncateString(res->getString("email"), 25),
            truncateString(res->getString("c_number"), 15),
            truncateString(roleText, 10)
        };
        printTableRow(row, widths);
    }

    if (!hasData) {
        centerText("No users found.");
    }

    printTableFooter(widths);
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    delete res;
    delete pstmt;
    delete con;
}

// ===== View all runners =====
void viewAllRunners() {
    clearScreen();
    printMenuTitle("All Runners");
    
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

    sql::PreparedStatement* pstmt = con->prepareStatement(
        "SELECT user_id, name, email, c_number FROM users WHERE role = 2"
    );
    sql::ResultSet* res = pstmt->executeQuery();

    std::vector<std::pair<std::string, int>> columns = {
        {"ID", 5},
        {"Name", 20},
        {"Email", 25},
        {"Contact", 15}
    };
    std::vector<int> widths = {5, 20, 25, 15};
    
    printTableHeader(columns);

    bool hasData = false;
    while (res->next()) {
        hasData = true;
        std::vector<std::string> row = {
            std::to_string(res->getInt("user_id")),
            truncateString(res->getString("name"), 20),
            truncateString(res->getString("email"), 25),
            truncateString(res->getString("c_number"), 15)
        };
        printTableRow(row, widths);
    }

    if (!hasData) {
        centerText("No runners found.");
    }

    printTableFooter(widths);
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    delete res;
    delete pstmt;
    delete con;
}

// ===== View all errands =====
void viewAllErrands() {
    clearScreen();
    printMenuTitle("All Errands");
    
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

    std::vector<std::pair<std::string, int>> columns = {
        {"ID", 5},
        {"Requester", 15},
        {"Runner", 15},
        {"Description", 25},
        {"Pickup", 15},
        {"Dropoff", 15},
        {"Distance", 10},
        {"Status", 12},
        {"Created At", 20}
    };
    std::vector<int> widths = {5, 15, 15, 25, 15, 15, 10, 12, 20};
    
    printTableHeader(columns);

    bool hasData = false;
    while (res->next()) {
        hasData = true;
        std::string runnerName = res->isNull("runner") ? "Unassigned" : res->getString("runner");
        std::ostringstream distStream;
        distStream << std::fixed << std::setprecision(2) << res->getDouble("distance");
        std::string status = res->getString("status");

        // Prepare row data
        std::vector<std::string> row = {
            std::to_string(res->getInt("errand_id")),
            truncateString(res->getString("requester"), 15),
            truncateString(runnerName, 15),
            truncateString(res->getString("description"), 25),
            truncateString(res->getString("pickup_loc"), 15),
            truncateString(res->getString("dropoff_loc"), 15),
            distStream.str(),
            truncateString(status, 12),
            truncateString(res->getString("created_at"), 20)
        };

        // Print row with colored status
        int width = getConsoleWidth();
        int totalWidth = 0;
        for (int w : widths) {
            totalWidth += w;
        }
        totalWidth += static_cast<int>(widths.size()) + 1;
        
        int padding = (width - totalWidth) / 2;
        if (padding > 0) std::cout << std::string(padding, ' ');
        
        setColor(COLOR_CYAN);
        std::cout << "|";
        resetColor();
        
        // Print each column
        for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
            std::string val = row[i];
            if (val.length() > static_cast<size_t>(widths[i])) {
                val = val.substr(0, widths[i] - 3) + "...";
            }
            
            // Color status column (index 7)
            if (i == 7) {
                int statusColor = COLOR_LIGHT_GRAY; // default
                if (status == "Pending") {
                    statusColor = COLOR_YELLOW;
                }
                else if (status == "Assigned") {
                    statusColor = COLOR_CYAN;
                }
                else if (status == "Completed") {
                    statusColor = COLOR_GREEN;
                }
                setColor(statusColor);
                std::cout << std::left << std::setw(widths[i]) << val;
                resetColor();
            }
            else {
                std::cout << std::left << std::setw(widths[i]) << val;
            }
            
            setColor(COLOR_CYAN);
            std::cout << "|";
            resetColor();
        }
        std::cout << "\n";
    }

    if (!hasData) {
        centerText("No errands found.");
    }

    printTableFooter(widths);
    
    // Print color legend
    std::cout << "\n";
    centerText("Status Color Legend:");
    std::cout << "\n";
    
    // Pending - Yellow
    int consoleWidth = getConsoleWidth();
    std::string pendingText = "Pending   - ";
    std::string pendingDesc = "Waiting for assignment";
    int pendingTotalLen = static_cast<int>(pendingText.length() + pendingDesc.length());
    int pendingPadding = (consoleWidth - pendingTotalLen) / 2;
    if (pendingPadding > 0) std::cout << std::string(pendingPadding, ' ');
    std::cout << pendingText;
    printColored(pendingDesc, COLOR_YELLOW);
    std::cout << "\n";
    
    // Assigned - Cyan
    std::string assignedText = "Assigned  - ";
    std::string assignedDesc = "Currently in progress";
    int assignedTotalLen = static_cast<int>(assignedText.length() + assignedDesc.length());
    int assignedPadding = (consoleWidth - assignedTotalLen) / 2;
    if (assignedPadding > 0) std::cout << std::string(assignedPadding, ' ');
    std::cout << assignedText;
    printColored(assignedDesc, COLOR_CYAN);
    std::cout << "\n";
    
    // Completed - Green
    std::string completedText = "Completed - ";
    std::string completedDesc = "Successfully finished";
    int completedTotalLen = static_cast<int>(completedText.length() + completedDesc.length());
    int completedPadding = (consoleWidth - completedTotalLen) / 2;
    if (completedPadding > 0) std::cout << std::string(completedPadding, ' ');
    std::cout << completedText;
    printColored(completedDesc, COLOR_GREEN);
    std::cout << "\n";
    
    std::cout << "\nPress Enter to continue...";
    std::cin.get();

    delete res;
    delete pstmt;
    delete con;
}

// ===== Update errand status =====
void updateErrandStatusAdmin() {
    viewAllErrands();
    int errandId;
    
    while (true) {
        errandId = getCenteredIntInput("Enter the ID of the errand to update (or 0 to go back): ");
        if (errandId == 0) return; // Back to menu
        if (errandId > 0) break;
        else centerText("Invalid input! Please enter a valid errand ID or 0 to go back.");
    }

    std::string newStatus;
    while (true) {
        newStatus = getCenteredInput("Enter new status (Pending, Assigned, Completed, or 'back' to cancel): ");

        if (newStatus == "back" || newStatus == "Back" || newStatus == "BACK") {
            centerText("Status update cancelled.");
            return;
        }
        if (newStatus == "Pending" || newStatus == "Assigned" || newStatus == "Completed") {
            break;
        }
        else {
            centerText("Invalid status! Please enter: Pending, Assigned, Completed, or 'back' to cancel.");
        }
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        // First, verify that the errand exists
        sql::PreparedStatement* checkStmt = con->prepareStatement(
            "SELECT errand_id FROM errands WHERE errand_id=?"
        );
        checkStmt->setInt(1, errandId);
        sql::ResultSet* checkRes = checkStmt->executeQuery();
        
        if (!checkRes->next()) {
            printError("Errand not found! Please enter a valid errand ID.");
            delete checkRes;
            delete checkStmt;
            delete con;
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        delete checkRes;
        delete checkStmt;

        // Update the errand status
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE errands SET status=? WHERE errand_id=?"
        );
        pstmt->setString(1, newStatus);
        pstmt->setInt(2, errandId);
        int updated = pstmt->executeUpdate();
        
        if (updated > 0) {
            printSuccess("Errand status updated successfully!");
        }
        else {
            printError("Failed to update errand status! Please try again.");
        }

        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        printError("Database error occurred! Please try again later.");
        std::cerr << "SQL Error: " << e.what() << std::endl;
        std::cerr << "Error Code: " << e.getErrorCode() << std::endl;
    }
    catch (std::exception& e) {
        printError("An unexpected error occurred! Please try again.");
        std::cerr << "Error: " << e.what() << std::endl;
    }
    catch (...) {
        printError("A critical error occurred! Please contact support.");
        std::cerr << "Unknown error occurred." << std::endl;
    }
    
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    try {
        viewAllErrands();
    }
    catch (...) {
        // If viewAllErrands fails, just continue without showing error
        // to prevent infinite error loop
    }
}

// ===== View pending errands only =====
void viewPendingErrands() {
    clearScreen();
    printMenuTitle("Pending Errands");
    
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
            "WHERE e.status = 'Pending' "
            "ORDER BY e.created_at DESC"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Requester", 15},
            {"Runner", 15},
            {"Description", 25},
            {"Pickup", 15},
            {"Dropoff", 15},
            {"Distance", 10},
            {"Status", 12},
            {"Created At", 20}
        };
        std::vector<int> widths = {5, 15, 15, 25, 15, 15, 10, 12, 20};
        
        printTableHeader(columns);

        bool hasData = false;
        while (res->next()) {
            hasData = true;
            std::string runnerName = res->isNull("runner") ? "Unassigned" : res->getString("runner");
            std::ostringstream distStream;
            distStream << std::fixed << std::setprecision(2) << res->getDouble("distance");

            std::vector<std::string> row = {
                std::to_string(res->getInt("errand_id")),
                truncateString(res->getString("requester"), 15),
                truncateString(runnerName, 15),
                truncateString(res->getString("description"), 25),
                truncateString(res->getString("pickup_loc"), 15),
                truncateString(res->getString("dropoff_loc"), 15),
                distStream.str(),
                truncateString(res->getString("status"), 12),
                truncateString(res->getString("created_at"), 20)
            };
            printTableRow(row, widths);
        }

        if (!hasData) {
            centerText("No pending errands found.");
        }

        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        std::cin.get();

        delete res;
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        printError("Database error occurred!");
        std::cerr << "SQL Error: " << e.what() << std::endl;
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    catch (...) {
        printError("An error occurred while loading errands!");
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
}

// ===== Assign errand to runner =====
void assignErrandToRunner() {
    viewPendingErrands();
    int errandId, runnerId;
    
    while (true) {
        errandId = getCenteredIntInput("Enter the ID of the errand to assign (or 0 to go back): ");
        if (errandId == 0) return; // Back to menu
        if (errandId > 0) break;
        else centerText("Invalid input! Please enter a valid errand ID or 0 to go back.");
    }
    
    // Display available runners
    try {
        clearScreen();
        printMenuTitle("Available Runners");
        
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        sql::PreparedStatement* listStmt = con->prepareStatement(
            "SELECT user_id, name, email, c_number FROM users WHERE role = 2"
        );
        sql::ResultSet* listRes = listStmt->executeQuery();

        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Name", 20},
            {"Email", 25},
            {"Contact", 15}
        };
        std::vector<int> widths = {5, 20, 25, 15};
        
        printTableHeader(columns);

        bool hasRunners = false;
        while (listRes->next()) {
            hasRunners = true;
            std::vector<std::string> row = {
                std::to_string(listRes->getInt("user_id")),
                truncateString(listRes->getString("name"), 20),
                truncateString(listRes->getString("email"), 25),
                truncateString(listRes->getString("c_number"), 15)
            };
            printTableRow(row, widths);
        }

        if (!hasRunners) {
            centerText("No runners found. Please register a runner first.");
            delete listRes;
            delete listStmt;
            delete con;
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }

        printTableFooter(widths);
        std::cout << "\n";
        
        delete listRes;
        delete listStmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        printError("Database error occurred while loading runners!");
        std::cerr << "SQL Error: " << e.what() << std::endl;
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        return;
    }
    catch (...) {
        printError("An error occurred while loading runners!");
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        return;
    }
    
    while (true) {
        runnerId = getCenteredIntInput("Enter the runner's user ID (or 0 to go back): ");
        if (runnerId == 0) return; // Back to menu
        if (runnerId > 0) break;
        else centerText("Invalid input! Please enter a valid runner ID or 0 to go back.");
    }

    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
        con->setSchema("erms");

        // Verify errand exists and is Pending
        sql::PreparedStatement* checkErrandStmt = con->prepareStatement(
            "SELECT errand_id, status FROM errands WHERE errand_id=?"
        );
        checkErrandStmt->setInt(1, errandId);
        sql::ResultSet* checkErrandRes = checkErrandStmt->executeQuery();
        
        if (!checkErrandRes->next()) {
            printError("Errand not found! Please enter a valid errand ID.");
            delete checkErrandRes;
            delete checkErrandStmt;
            delete con;
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        
        std::string errandStatus = checkErrandRes->getString("status");
        if (errandStatus != "Pending") {
            printError("This errand is not pending! Only pending errands can be assigned to runners.");
            delete checkErrandRes;
            delete checkErrandStmt;
            delete con;
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        delete checkErrandRes;
        delete checkErrandStmt;

        // Verify runner exists and has runner role
        sql::PreparedStatement* checkRunnerStmt = con->prepareStatement(
            "SELECT user_id, role FROM users WHERE user_id=? AND role=2"
        );
        checkRunnerStmt->setInt(1, runnerId);
        sql::ResultSet* checkRunnerRes = checkRunnerStmt->executeQuery();
        
        if (!checkRunnerRes->next()) {
            printError("Runner not found or user is not a runner! Please enter a valid runner ID.");
            delete checkRunnerRes;
            delete checkRunnerStmt;
            delete con;
            std::cout << "\nPress Enter to continue...";
            std::cin.get();
            return;
        }
        delete checkRunnerRes;
        delete checkRunnerStmt;

        // Assign errand to runner
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "UPDATE errands SET runner_id=?, status='Assigned' WHERE errand_id=?"
        );
        pstmt->setInt(1, runnerId);
        pstmt->setInt(2, errandId);
        int updated = pstmt->executeUpdate();
        
        if (updated > 0) {
            printSuccess("Errand assigned to runner successfully!");
        }
        else {
            printError("Failed to assign errand! Please try again.");
        }
        
        delete pstmt;
        delete con;
    }
    catch (sql::SQLException& e) {
        printError("Database error occurred! Please try again later.");
        std::cerr << "SQL Error: " << e.what() << std::endl;
        std::cerr << "Error Code: " << e.getErrorCode() << std::endl;
    }
    catch (std::exception& e) {
        printError("An unexpected error occurred! Please try again.");
        std::cerr << "Error: " << e.what() << std::endl;
    }
    catch (...) {
        printError("A critical error occurred! Please contact support.");
        std::cerr << "Unknown error occurred." << std::endl;
    }
    
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    try {
        viewPendingErrands();
    }
    catch (...) {
        // If viewPendingErrands fails, just continue without showing error
    }
}

// ===== Register runner manually =====
void registerRunner() {
    std::string enrollmentPassword;
    enrollmentPassword = getCenteredInput("Enter runner enrollment password (or type 'back' to cancel): ");
    if (enrollmentPassword == "back" || enrollmentPassword == "Back" || enrollmentPassword == "BACK") {
        centerText("Registration cancelled.");
        return;
    }
    if (enrollmentPassword != RUNNER_ENROLLMENT_PASSWORD) {
        centerText("Invalid enrollment password. Registration aborted.");
        return;
    }

    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

    std::string username, password, email, contactNumber;

        // ===== Username =====
        while (true) {
            username = getCenteredInput("Enter username (2-50 chars, letters & numbers, no spaces) or 'back' to cancel: ");

            if (username == "back" || username == "Back" || username == "BACK") {
                centerText("Registration cancelled.");
                delete con;
                return;
            }

            if (!isValidName(username)) {
                centerText("Invalid username! Re-enter.");
                continue;
            }

            // Check duplicate username
            sql::PreparedStatement* checkUserStmt = con->prepareStatement(
                "SELECT COUNT(*) AS count FROM users WHERE name=?"
            );
            checkUserStmt->setString(1, username);
            sql::ResultSet* resUser = checkUserStmt->executeQuery();
            resUser->next();
            if (resUser->getInt("count") > 0) {
                centerText("Username already taken! Please enter another.");
                delete resUser;
                delete checkUserStmt;
                continue;
            }

            delete resUser;
            delete checkUserStmt;
            break; // valid & unique
        }

        // ===== Password =====
        password = getCenteredInput("Enter password (8+ chars, uppercase, lowercase, number and symbol): ");
        while (!isValidPassword(password)) {
            centerText("Password must be 8+ chars, include uppercase, lowercase, number, and symbol. Re-enter.");
            password = getCenteredInput("Enter password: ");
        }

        std::string confirmPassword;
        confirmPassword = getCenteredInput("Confirm password: ");
        while (confirmPassword != password) {
            centerText("Passwords do not match! Re-enter confirmation.");
            confirmPassword = getCenteredInput("Confirm password: ");
        }

        // ===== Email =====
        while (true) {
            email = getCenteredInput("Enter email (or 'back' to cancel): ");

            if (email == "back" || email == "Back" || email == "BACK") {
                centerText("Registration cancelled.");
                delete con;
                return;
            }

            if (!isValidEmail(email)) {
                centerText("Invalid email format. Re-enter.");
                continue;
            }

            // Check duplicate email
            sql::PreparedStatement* checkEmailStmt = con->prepareStatement(
                "SELECT COUNT(*) AS count FROM users WHERE email=?"
            );
            checkEmailStmt->setString(1, email);
            sql::ResultSet* resEmail = checkEmailStmt->executeQuery();
            resEmail->next();
            if (resEmail->getInt("count") > 0) {
                centerText("Email already registered! Enter another email.");
                delete resEmail;
                delete checkEmailStmt;
                continue;
            }

            delete resEmail;
            delete checkEmailStmt;
            break; // Valid & unique email
        }

        // ===== Contact Number =====
        while (true) {
            contactNumber = getCenteredInput("Enter contact number (with optional +countrycode, or 'back' to cancel): ");
            
            if (contactNumber == "back" || contactNumber == "Back" || contactNumber == "BACK") {
                centerText("Registration cancelled.");
                delete con;
                return;
            }
            
            if (!isValidPhoneIntl(contactNumber)) {
                centerText("Invalid phone! Must be 7-15 digits, can start with +. Re-enter.");
                continue;
            }

            // Check duplicate
            sql::PreparedStatement* checkStmt = con->prepareStatement(
                "SELECT c_number FROM users"
            );
            sql::ResultSet* res = checkStmt->executeQuery();
            bool duplicate = false;
            std::string normalizedInput = normalizePhone(contactNumber);
            while (res->next()) {
                std::string existing = normalizePhone(res->getString("c_number"));
                if (normalizedInput == existing) { duplicate = true; break; }
            }

            delete res;
            delete checkStmt;

            if (duplicate) {
                centerText("This contact number is already registered! Enter another number.");
                continue;
            }

            break; // Valid & unique
        }

        std::string hashedPassword = hashPassword(password);

        // Insert user
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "INSERT INTO users(name, password, email, c_number, role) VALUES (?, ?, ?, ?, 2)"
        );
        pstmt->setString(1, username);
        pstmt->setString(2, hashedPassword);
        pstmt->setString(3, email);
        pstmt->setString(4, contactNumber);
        pstmt->execute();

        printSuccess("Runner registered successfully!");
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();

        delete pstmt;
        delete con;
}

// ===== Top Runners by Income =====
void showTopRunnersByIncome() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "SELECT u.user_id, u.name, COALESCE(SUM(q.runner_share), 0) AS total_income "
            "FROM users u "
            "INNER JOIN errands e ON u.user_id = e.runner_id "
            "INNER JOIN quotations q ON e.errand_id = q.errand_id "
            "WHERE u.role = 2 AND q.status = 'Paid' "
            "GROUP BY u.user_id, u.name "
            "ORDER BY total_income DESC "
            "LIMIT 5"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        clearScreen();
        printMenuTitle("Top 5 Runners by Income");
        
        bool hasData = false;
        while (res->next()) {
            hasData = true;
            std::string runnerName = res->getString("name");
            double income = res->getDouble("total_income");
            displayStarChart(runnerName, income);
        }

        if (!hasData) {
            centerText("No runners with income data found.");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();

        delete res;
        delete pstmt;
        delete con;
}

// ===== Top Runners by Errands =====
void showTopRunnersByErrands() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "SELECT u.user_id, u.name, COUNT(e.errand_id) AS total_errands "
            "FROM users u "
            "INNER JOIN errands e ON u.user_id = e.runner_id "
            "WHERE u.role = 2 AND e.status = 'Completed' "
            "GROUP BY u.user_id, u.name "
            "ORDER BY total_errands DESC "
            "LIMIT 5"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        clearScreen();
        printMenuTitle("Top 5 Runners by Errands");
        
        bool hasData = false;
        while (res->next()) {
            hasData = true;
            std::string runnerName = res->getString("name");
            int errandCount = res->getInt("total_errands");
            int stars = errandCount; // 1 star per errand for visualization
            std::string starString(stars, '*');
            centerText(runnerName + " : " + std::to_string(errandCount) + " errands (" + starString + ")");
        }

        if (!hasData) {
            centerText("No runners with completed errands found.");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();

        delete res;
        delete pstmt;
        delete con;
}

// ===== Monthly Errands Chart =====
void showMonthlyErrandsChart() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

    // Query to get monthly errands count
    sql::PreparedStatement* pstmt = con->prepareStatement(
        "SELECT YEAR(created_at) AS year, "
        "MONTH(created_at) AS month, "
        "COUNT(*) AS errand_count "
        "FROM errands "
        "GROUP BY YEAR(created_at), MONTH(created_at) "
        "ORDER BY year ASC, month ASC"
    );
    sql::ResultSet* res = pstmt->executeQuery();

    clearScreen();
    printMenuTitle("Monthly Errands Chart");

    // First pass: collect data and find max count for scaling
    std::vector<std::tuple<int, int, int>> monthlyData; // year, month, count
    int maxCount = 0;
    bool hasData = false;

    while (res->next()) {
        hasData = true;
        int year = res->getInt("year");
        int month = res->getInt("month");
        int count = res->getInt("errand_count");
        monthlyData.push_back(std::make_tuple(year, month, count));
        if (count > maxCount) {
            maxCount = count;
        }
    }

    if (!hasData) {
        centerText("No errands found in the database.");
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        delete res;
        delete pstmt;
        delete con;
        return;
    }

    // Month names array
    const std::string monthNames[] = {
        "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    std::cout << "\n";
    centerText("Monthly Errands Ordered (All Time)");
    std::cout << "\n";
    centerText("========================================");
    std::cout << "\n";

    // Second pass: display the bar chart
    for (const auto& data : monthlyData) {
        int year = std::get<0>(data);
        int month = std::get<1>(data);
        int count = std::get<2>(data);

        // Create month label
        std::string monthLabel = monthNames[month] + " " + std::to_string(year);
        
        // Each # represents 1 parcel (no scaling)
        std::string barString(count, '#');

        // Format and display
        std::ostringstream line;
        line << monthLabel << " : " << count << " errands (" << barString << ")";
        centerText(line.str());
    }

    std::cout << "\n";
    centerText("========================================");
    std::cout << "\n";
    centerText("Note: Each # represents 1 parcel");
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    std::cin.get();

    delete res;
    delete pstmt;
    delete con;
}

// ===== System Revenue =====
void showSystemRevenue() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "SELECT COALESCE(SUM(system_fee), 0) AS total_revenue "
            "FROM quotations "
            "WHERE status = 'Paid'"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        clearScreen();
        printMenuTitle("System Revenue");
        
        if (res->next()) {
            double revenue = res->getDouble("total_revenue");
            if (revenue > 0) {
                std::ostringstream revStream;
                revStream << std::fixed << std::setprecision(2) << revenue;
                centerText("Total System Revenue: RM " + revStream.str());
            }
            else {
                centerText("No system revenue found. No paid quotations yet.");
            }
        }
        else {
            centerText("No revenue data available.");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();

        delete res;
        delete pstmt;
        delete con;
}

// ===== All Runner Revenue =====
void showAllRunnerRevenue() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "SELECT COALESCE(SUM(runner_share), 0) AS total_revenue "
            "FROM quotations "
            "WHERE status = 'Paid'"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        clearScreen();
        printMenuTitle("All Runner Revenue");
        
        if (res->next()) {
            double revenue = res->getDouble("total_revenue");
            if (revenue > 0) {
                std::ostringstream revStream;
                revStream << std::fixed << std::setprecision(2) << revenue;
                centerText("Total Runner Revenue: RM " + revStream.str());
            }
            else {
                centerText("No runner revenue found. No paid quotations yet.");
            }
        }
        else {
            centerText("No revenue data available.");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();

        delete res;
        delete pstmt;
        delete con;
}

// ===== Save Report Snapshot =====
void saveReportSnapshot() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

        // Get top runner by income
        std::string topRunnerName = "N/A";
        sql::PreparedStatement* pstmt1 = con->prepareStatement(
            "SELECT u.name, COALESCE(SUM(q.runner_share), 0) AS total_income "
            "FROM users u "
            "INNER JOIN errands e ON u.user_id = e.runner_id "
            "INNER JOIN quotations q ON e.errand_id = q.errand_id "
            "WHERE u.role = 2 AND q.status = 'Paid' "
            "GROUP BY u.user_id, u.name "
            "ORDER BY total_income DESC "
            "LIMIT 1"
        );
        sql::ResultSet* res1 = pstmt1->executeQuery();
        if (res1->next()) {
            topRunnerName = res1->getString("name");
        }
        delete res1;
        delete pstmt1;

        // Get system revenue
        double systemRevenue = 0.0;
        sql::PreparedStatement* pstmt2 = con->prepareStatement(
            "SELECT COALESCE(SUM(system_fee), 0) AS total_revenue "
            "FROM quotations "
            "WHERE status = 'Paid'"
        );
        sql::ResultSet* res2 = pstmt2->executeQuery();
        if (res2->next()) {
            systemRevenue = res2->getDouble("total_revenue");
        }
        delete res2;
        delete pstmt2;

        // Get total runner revenue
        double totalRunnerRevenue = 0.0;
        sql::PreparedStatement* pstmt3 = con->prepareStatement(
            "SELECT COALESCE(SUM(runner_share), 0) AS total_revenue "
            "FROM quotations "
            "WHERE status = 'Paid'"
        );
        sql::ResultSet* res3 = pstmt3->executeQuery();
        if (res3->next()) {
            totalRunnerRevenue = res3->getDouble("total_revenue");
        }
        delete res3;
        delete pstmt3;

        // Get total errands count
        int totalErrands = 0;
        sql::PreparedStatement* pstmt4 = con->prepareStatement(
            "SELECT COUNT(*) AS total_errands FROM errands"
        );
        sql::ResultSet* res4 = pstmt4->executeQuery();
        if (res4->next()) {
            totalErrands = res4->getInt("total_errands");
        }
        delete res4;
        delete pstmt4;

        // Insert report snapshot
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "INSERT INTO reports (total_errands, top_runner, system_revenue, total_runner_revenue, report_date) "
            "VALUES (?, ?, ?, ?, CURDATE())"
        );
        pstmt->setInt(1, totalErrands);
        pstmt->setString(2, topRunnerName);
        pstmt->setDouble(3, systemRevenue);
        pstmt->setDouble(4, totalRunnerRevenue);
        pstmt->execute();

        clearScreen();
        printMenuTitle("Report Snapshot Saved Successfully");
        std::ostringstream sysRevStream, runnerRevStream;
        sysRevStream << std::fixed << std::setprecision(2) << systemRevenue;
        runnerRevStream << std::fixed << std::setprecision(2) << totalRunnerRevenue;
        
        centerText("Total Errands: " + std::to_string(totalErrands));
        centerText("Top Runner: " + topRunnerName);
        centerText("System Revenue: RM " + sysRevStream.str());
        centerText("Total Runner Revenue: RM " + runnerRevStream.str());
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();

        delete pstmt;
        delete con;
}

// ===== View All Receipts =====
void viewAllReceipts() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "SELECT p.payment_id, p.transaction_id, p.quote_id, p.errand_id, p.price, p.pay_status, p.payment_method, "
            "q.base_price_per_km, q.distance_km, q.runner_percentage, q.system_percentage, "
            "q.runner_share, q.system_fee, p.created_at "
            "FROM payments p "
            "INNER JOIN quotations q ON p.quote_id = q.quote_id "
            "ORDER BY p.created_at DESC"
        );
        sql::ResultSet* res = pstmt->executeQuery();

        clearScreen();
        printMenuTitle("All Payment Receipts");
        bool hasReceipts = false;

        while (res->next()) {
            hasReceipts = true;
            std::string transactionId = formatTransactionId(res->getString("transaction_id"));
            int quoteId = res->getInt("quote_id");
            int errandId = res->getInt("errand_id");
            double base = res->getDouble("base_price_per_km");
            double distance = res->getDouble("distance_km");
            double runnerPerc = res->getDouble("runner_percentage");
            double systemPerc = res->getDouble("system_percentage");
            double total = res->getDouble("price");
            double runnerFee = res->getDouble("runner_share");
            double systemFee = res->getDouble("system_fee");
            std::string payStatus = res->getString("pay_status");
            std::string paymentMethod = res->getString("payment_method");
            
            std::ostringstream baseStream, totalStream, runnerFeeStream, systemFeeStream;
            baseStream << std::fixed << std::setprecision(2) << base;
            totalStream << std::fixed << std::setprecision(2) << total;
            runnerFeeStream << std::fixed << std::setprecision(2) << runnerFee;
            systemFeeStream << std::fixed << std::setprecision(2) << systemFee;

            // ================= RECEIPT =================
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
            centerText("Method         : " + paymentMethod);
            centerText("Payment Date   : " + res->getString("created_at"));
            centerText("==========================================");
            std::cout << "\n";
        }

        if (!hasReceipts) {
            centerText("No receipts found.");
        }
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();

        delete res;
        delete pstmt;
        delete con;
}

// ===== Search Users =====
void searchUsers() {
    clearScreen();
    printMenuTitle("Search Users");
    printHeader("SEARCH BY");
    centerText("1. Name");
    centerText("2. Email");
    centerText("3. Role");
    printHeader("");
    std::cout << "\n";
    
    int searchType = getMenuChoice(1, 3);
    
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");
    
    sql::PreparedStatement* pstmt = nullptr;
    
    if (searchType == 1) {
        std::string name;
            while (true) {
                name = getCenteredInput("Enter name (partial match, or 'back' to cancel): ");
                if (name == "back" || name == "Back" || name == "BACK") {
                    delete con;
                    return;
                }
                if (!name.empty() && name.length() >= 1) {
                    break;
                }
                else {
                    centerText("Name cannot be empty! Please enter a name.");
                }
            }
            pstmt = con->prepareStatement(
                "SELECT user_id, name, email, c_number, role FROM users WHERE name LIKE ?"
            );
            pstmt->setString(1, "%" + name + "%");
        }
        else if (searchType == 2) {
            std::string email;
            while (true) {
                email = getCenteredInput("Enter email (partial match, or 'back' to cancel): ");
                if (email == "back" || email == "Back" || email == "BACK") {
                    delete con;
                    return;
                }
                if (!email.empty() && email.length() >= 1) {
                    break;
                }
                else {
                    centerText("Email cannot be empty! Please enter an email.");
                }
            }
            pstmt = con->prepareStatement(
                "SELECT user_id, name, email, c_number, role FROM users WHERE email LIKE ?"
            );
            pstmt->setString(1, "%" + email + "%");
        }
        else if (searchType == 3) {
            int role;
            while (true) {
                role = getCenteredIntInput("Enter role (0=User, 1=Admin, 2=Runner, or -1 to go back): ");
                if (role == -1) {
                    delete con;
                    return;
                }
                if (role >= 0 && role <= 2) {
                    break;
                }
                else {
                    centerText("Invalid input! Please enter 0, 1, 2, or -1 to go back.");
                }
            }
            pstmt = con->prepareStatement(
                "SELECT user_id, name, email, c_number, role FROM users WHERE role = ?"
            );
            pstmt->setInt(1, role);
        }
        
        sql::ResultSet* res = pstmt->executeQuery();
        
        clearScreen();
        printMenuTitle("Search Results - Users");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Name", 20},
            {"Email", 25},
            {"Contact", 15},
            {"Role", 10}
        };
        std::vector<int> widths = {5, 20, 25, 15, 10};
        
        printTableHeader(columns);
        
        bool hasResults = false;
        while (res->next()) {
            hasResults = true;
            int role = res->getInt("role");
            std::string roleText = (role == 0) ? "User" : (role == 1) ? "Admin" : "Runner";
            
            std::vector<std::string> row = {
                std::to_string(res->getInt("user_id")),
                truncateString(res->getString("name"), 20),
                truncateString(res->getString("email"), 25),
                truncateString(res->getString("c_number"), 15),
                truncateString(roleText, 10)
            };
            printTableRow(row, widths);
        }
        
        if (!hasResults) {
            centerText("No users found.");
        }
        
        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        // Clear any remaining input in buffer
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        
        delete res;
        delete pstmt;
        delete con;
}

// ===== Search Errands =====
void searchErrands() {
    clearScreen();
    printMenuTitle("Search Errands");
    printHeader("SEARCH BY");
    centerText("1. Errand ID");
    centerText("2. Description (keyword)");
    centerText("3. Requester Name");
    centerText("4. Runner Name");
    centerText("5. Status");
    printHeader("");
    std::cout << "\n";
    
    int searchType = getMenuChoice(1, 5);

    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");
    
    sql::PreparedStatement* pstmt = nullptr;
    
    if (searchType == 1) {
        int errandId;
            while (true) {
                errandId = getCenteredIntInput("Enter Errand ID (or 0 to go back): ");
                if (errandId == 0) {
                    delete con;
                    return;
                }
                if (errandId > 0) break;
                else centerText("Invalid input! Please enter a valid errand ID or 0 to go back.");
            }
            pstmt = con->prepareStatement(
                "SELECT e.errand_id, u.name AS requester, e.description, e.pickup_loc, "
                "e.dropoff_loc, e.distance, e.status, e.created_at, r.name AS runner "
                "FROM errands e "
                "LEFT JOIN users u ON e.requester_id = u.user_id "
                "LEFT JOIN users r ON e.runner_id = r.user_id "
                "WHERE e.errand_id = ?"
            );
        pstmt->setInt(1, errandId);
    }
    else if (searchType == 2) {
        std::string keyword;
            while (true) {
                keyword = getCenteredInput("Enter keyword (or 'back' to cancel): ");
                if (keyword == "back" || keyword == "Back" || keyword == "BACK") {
                    delete con;
                    return;
                }
                if (!keyword.empty() && keyword.length() >= 1) {
                    break;
                }
                else {
                    centerText("Keyword cannot be empty! Please enter a keyword.");
                }
            }
        pstmt = con->prepareStatement(
            "SELECT e.errand_id, u.name AS requester, e.description, e.pickup_loc, "
            "e.dropoff_loc, e.distance, e.status, e.created_at, r.name AS runner "
            "FROM errands e "
            "LEFT JOIN users u ON e.requester_id = u.user_id "
            "LEFT JOIN users r ON e.runner_id = r.user_id "
            "WHERE e.description LIKE ?"
        );
        pstmt->setString(1, "%" + keyword + "%");
    }
    else if (searchType == 3) {
        std::string requesterName;
        while (true) {
            requesterName = getCenteredInput("Enter requester name (or 'back' to cancel): ");
            if (requesterName == "back" || requesterName == "Back" || requesterName == "BACK") {
                delete con;
                return;
            }
            if (!requesterName.empty() && requesterName.length() >= 1) {
                break;
            }
            else {
                centerText("Requester name cannot be empty! Please enter a name.");
            }
        }
        pstmt = con->prepareStatement(
            "SELECT e.errand_id, u.name AS requester, e.description, e.pickup_loc, "
            "e.dropoff_loc, e.distance, e.status, e.created_at, r.name AS runner "
            "FROM errands e "
            "LEFT JOIN users u ON e.requester_id = u.user_id "
            "LEFT JOIN users r ON e.runner_id = r.user_id "
            "WHERE u.name LIKE ?"
        );
        pstmt->setString(1, "%" + requesterName + "%");
    }
    else if (searchType == 4) {
        std::string runnerName;
        while (true) {
            runnerName = getCenteredInput("Enter runner name (or 'back' to cancel): ");
            if (runnerName == "back" || runnerName == "Back" || runnerName == "BACK") {
                delete con;
                return;
            }
            if (!runnerName.empty() && runnerName.length() >= 1) {
                break;
            }
            else {
                centerText("Runner name cannot be empty! Please enter a name.");
            }
        }
        pstmt = con->prepareStatement(
            "SELECT e.errand_id, u.name AS requester, e.description, e.pickup_loc, "
            "e.dropoff_loc, e.distance, e.status, e.created_at, r.name AS runner "
            "FROM errands e "
            "LEFT JOIN users u ON e.requester_id = u.user_id "
            "LEFT JOIN users r ON e.runner_id = r.user_id "
            "WHERE r.name LIKE ?"
        );
        pstmt->setString(1, "%" + runnerName + "%");
    }
    else if (searchType == 5) {
        std::string status;
        while (true) {
            status = getCenteredInput("Enter status (Pending/Assigned/Completed, or 'back' to cancel): ");
            if (status == "back" || status == "Back" || status == "BACK") {
                delete con;
                return;
            }
            if (status == "Pending" || status == "Assigned" || status == "Completed") {
                break;
            }
            else {
                centerText("Invalid status! Please enter: Pending, Assigned, Completed, or 'back' to cancel.");
            }
        }
        pstmt = con->prepareStatement(
            "SELECT e.errand_id, u.name AS requester, e.description, e.pickup_loc, "
            "e.dropoff_loc, e.distance, e.status, e.created_at, r.name AS runner "
            "FROM errands e "
            "LEFT JOIN users u ON e.requester_id = u.user_id "
            "LEFT JOIN users r ON e.runner_id = r.user_id "
            "WHERE e.status = ?"
        );
        pstmt->setString(1, status);
    }
        
        sql::ResultSet* res = pstmt->executeQuery();
        
        clearScreen();
        printMenuTitle("Search Results - Errands");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Requester", 15},
            {"Runner", 15},
            {"Description", 25},
            {"Pickup", 15},
            {"Dropoff", 15},
            {"Distance", 10},
            {"Status", 12},
            {"Created At", 20}
        };
        std::vector<int> widths = {5, 15, 15, 25, 15, 15, 10, 12, 20};
        
        printTableHeader(columns);
        
        bool hasResults = false;
        while (res->next()) {
            hasResults = true;
            std::string runnerName = res->isNull("runner") ? "Unassigned" : res->getString("runner");
            std::ostringstream distStream;
            distStream << std::fixed << std::setprecision(2) << res->getDouble("distance");
            
            std::vector<std::string> row = {
                std::to_string(res->getInt("errand_id")),
                truncateString(res->getString("requester"), 15),
                truncateString(runnerName, 15),
                truncateString(res->getString("description"), 25),
                truncateString(res->getString("pickup_loc"), 15),
                truncateString(res->getString("dropoff_loc"), 15),
                distStream.str(),
                truncateString(res->getString("status"), 12),
                truncateString(res->getString("created_at"), 20)
            };
            printTableRow(row, widths);
        }
        
        if (!hasResults) {
            centerText("No errands found.");
        }
        
        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        // Clear any remaining input in buffer
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        
        delete res;
        delete pstmt;
        delete con;
}

// ===== Search Receipts =====
void searchReceipts() {
    clearScreen();
    printMenuTitle("Search Receipts");
    printHeader("SEARCH BY");
    centerText("1. Transaction ID");
    centerText("2. Quotation ID");
    centerText("3. Errand ID");
    printHeader("");
    std::cout << "\n";
    
    int searchType = getMenuChoice(1, 3);
    
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");
    
        sql::PreparedStatement* pstmt = nullptr;
        
        if (searchType == 1) {
            std::string transactionId;
            while (true) {
                transactionId = getCenteredInput("Enter Transaction ID (or 'back' to cancel): ");
                if (transactionId == "back" || transactionId == "Back" || transactionId == "BACK") {
                    delete con;
                    return;
                }
                if (!transactionId.empty() && transactionId.length() >= 1) {
                    break;
                }
                else {
                    centerText("Transaction ID cannot be empty! Please enter a transaction ID.");
                }
            }
            pstmt = con->prepareStatement(
                "SELECT p.payment_id, p.transaction_id, p.quote_id, p.errand_id, p.price, p.pay_status, p.payment_method, "
                "q.base_price_per_km, q.distance_km, q.runner_percentage, q.system_percentage, "
                "q.runner_share, q.system_fee, p.created_at "
                "FROM payments p "
                "INNER JOIN quotations q ON p.quote_id = q.quote_id "
                "WHERE p.transaction_id LIKE ?"
            );
            pstmt->setString(1, "%" + transactionId + "%");
        }
        else if (searchType == 2) {
            int quoteId;
            while (true) {
                quoteId = getCenteredIntInput("Enter Quotation ID (or 0 to go back): ");
                if (quoteId == 0) {
                    delete con;
                    return;
                }
                if (quoteId > 0) break;
                else centerText("Invalid input! Please enter a valid quotation ID or 0 to go back.");
            }
            pstmt = con->prepareStatement(
                "SELECT p.payment_id, p.transaction_id, p.quote_id, p.errand_id, p.price, p.pay_status, p.payment_method, "
                "q.base_price_per_km, q.distance_km, q.runner_percentage, q.system_percentage, "
                "q.runner_share, q.system_fee, p.created_at "
                "FROM payments p "
                "INNER JOIN quotations q ON p.quote_id = q.quote_id "
                "WHERE p.quote_id = ?"
            );
            pstmt->setInt(1, quoteId);
        }
        else if (searchType == 3) {
            int errandId;
            while (true) {
                errandId = getCenteredIntInput("Enter Errand ID (or 0 to go back): ");
                if (errandId == 0) {
                    delete con;
                    return;
                }
                if (errandId > 0) break;
                else centerText("Invalid input! Please enter a valid errand ID or 0 to go back.");
            }
            pstmt = con->prepareStatement(
                "SELECT p.payment_id, p.transaction_id, p.quote_id, p.errand_id, p.price, p.pay_status, p.payment_method, "
                "q.base_price_per_km, q.distance_km, q.runner_percentage, q.system_percentage, "
                "q.runner_share, q.system_fee, p.created_at "
                "FROM payments p "
                "INNER JOIN quotations q ON p.quote_id = q.quote_id "
                "WHERE p.errand_id = ?"
            );
            pstmt->setInt(1, errandId);
        }
        
        sql::ResultSet* res = pstmt->executeQuery();
        
        clearScreen();
        printMenuTitle("Search Results - Receipts");
        bool hasResults = false;
        
        while (res->next()) {
            hasResults = true;
            std::string transactionId = formatTransactionId(res->getString("transaction_id"));
            int quoteId = res->getInt("quote_id");
            int errandId = res->getInt("errand_id");
            double base = res->getDouble("base_price_per_km");
            double distance = res->getDouble("distance_km");
            double runnerPerc = res->getDouble("runner_percentage");
            double systemPerc = res->getDouble("system_percentage");
            double total = res->getDouble("price");
            double runnerFee = res->getDouble("runner_share");
            double systemFee = res->getDouble("system_fee");
            std::string payStatus = res->getString("pay_status");
            std::string paymentMethod = res->getString("payment_method");
            
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
            centerText("Method         : " + paymentMethod);
            centerText("Payment Date   : " + res->getString("created_at"));
            centerText("==========================================");
            std::cout << "\n";
        }
        
        if (!hasResults) {
            centerText("No receipts found.");
        }
        std::cout << "\nPress Enter to continue...";
        // Clear any remaining input in buffer
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        
        delete res;
        delete pstmt;
        delete con;
}

// ===== Search Runners =====
void searchRunners() {
    clearScreen();
    printMenuTitle("Search Runners");
    printHeader("SEARCH BY");
    centerText("1. Name");
    centerText("2. Email");
    printHeader("");
    std::cout << "\n";
    
    int searchType = getMenuChoice(1, 2);

    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");
    
    sql::PreparedStatement* pstmt = nullptr;
    
    if (searchType == 1) {
        std::string name;
            while (true) {
                name = getCenteredInput("Enter name (partial match, or 'back' to cancel): ");
                if (name == "back" || name == "Back" || name == "BACK") {
                    delete con;
                    return;
                }
                if (!name.empty() && name.length() >= 1) {
                    break;
                }
                else {
                    centerText("Name cannot be empty! Please enter a name.");
                }
            }
        pstmt = con->prepareStatement(
            "SELECT user_id, name, email, c_number FROM users WHERE role = 2 AND name LIKE ?"
        );
        pstmt->setString(1, "%" + name + "%");
    }
    else if (searchType == 2) {
        std::string email;
            while (true) {
                email = getCenteredInput("Enter email (partial match, or 'back' to cancel): ");
                if (email == "back" || email == "Back" || email == "BACK") {
                    delete con;
                    return;
                }
                if (!email.empty() && email.length() >= 1) {
                    break;
                }
                else {
                    centerText("Email cannot be empty! Please enter an email.");
                }
            }
        pstmt = con->prepareStatement(
            "SELECT user_id, name, email, c_number FROM users WHERE role = 2 AND email LIKE ?"
        );
        pstmt->setString(1, "%" + email + "%");
    }
        
        sql::ResultSet* res = pstmt->executeQuery();
        
        clearScreen();
        printMenuTitle("Search Results - Runners");
        
        std::vector<std::pair<std::string, int>> columns = {
            {"ID", 5},
            {"Name", 20},
            {"Email", 25},
            {"Contact", 15}
        };
        std::vector<int> widths = {5, 20, 25, 15};
        
        printTableHeader(columns);
        
        bool hasResults = false;
        while (res->next()) {
            hasResults = true;
            std::vector<std::string> row = {
                std::to_string(res->getInt("user_id")),
                truncateString(res->getString("name"), 20),
                truncateString(res->getString("email"), 25),
                truncateString(res->getString("c_number"), 15)
            };
            printTableRow(row, widths);
        }
        
        if (!hasResults) {
            centerText("No runners found.");
        }
        
        printTableFooter(widths);
        std::cout << "\nPress Enter to continue...";
        // Clear any remaining input in buffer
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        
        delete res;
        delete pstmt;
        delete con;
}

// ===== Search Quotations =====
void searchQuotations() {
    clearScreen();
    printMenuTitle("Search Quotations");
    printHeader("SEARCH BY");
    centerText("1. Quotation ID");
    centerText("2. Errand ID");
    centerText("3. Status");
    printHeader("");
    std::cout << "\n";
    
    int searchType = getMenuChoice(1, 3);
    
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    sql::Connection* con = driver->connect("tcp://localhost:3306", "root", "");
    con->setSchema("erms");
    
    sql::PreparedStatement* pstmt = nullptr;
    
    if (searchType == 1) {
        int quoteId;
            while (true) {
                quoteId = getCenteredIntInput("Enter Quotation ID (or 0 to go back): ");
                if (quoteId == 0) {
                    delete con;
                    return;
                }
                if (quoteId > 0) break;
                else centerText("Invalid input! Please enter a valid quotation ID or 0 to go back.");
            }
            pstmt = con->prepareStatement(
                "SELECT q.quote_id, q.errand_id, q.base_price_per_km, q.distance_km, "
                "q.runner_percentage, q.system_percentage, q.runner_share, q.system_fee, q.status, q.transaction_id "
                "FROM quotations q WHERE q.quote_id = ?"
            );
            pstmt->setInt(1, quoteId);
        }
        else if (searchType == 2) {
            int errandId;
            while (true) {
                errandId = getCenteredIntInput("Enter Errand ID (or 0 to go back): ");
                if (errandId == 0) {
                    delete con;
                    return;
                }
                if (errandId > 0) break;
                else centerText("Invalid input! Please enter a valid errand ID or 0 to go back.");
            }
            pstmt = con->prepareStatement(
                "SELECT q.quote_id, q.errand_id, q.base_price_per_km, q.distance_km, "
                "q.runner_percentage, q.system_percentage, q.runner_share, q.system_fee, q.status, q.transaction_id "
                "FROM quotations q WHERE q.errand_id = ?"
            );
            pstmt->setInt(1, errandId);
        }
        else if (searchType == 3) {
            std::string status;
            while (true) {
                status = getCenteredInput("Enter status (Pending/Paid, or 'back' to cancel): ");
                if (status == "back" || status == "Back" || status == "BACK") {
                    delete con;
                    return;
                }
                if (status == "Pending" || status == "Paid") {
                    break;
                }
                else {
                    centerText("Invalid status! Please enter: Pending, Paid, or 'back' to cancel.");
                }
            }
            pstmt = con->prepareStatement(
                "SELECT q.quote_id, q.errand_id, q.base_price_per_km, q.distance_km, "
                "q.runner_percentage, q.system_percentage, q.runner_share, q.system_fee, q.status, q.transaction_id "
                "FROM quotations q WHERE q.status = ?"
            );
            pstmt->setString(1, status);
        }
        
        sql::ResultSet* res = pstmt->executeQuery();
        
        clearScreen();
        printMenuTitle("Search Results - Quotations");
        
        bool hasResults = false;
        while (res->next()) {
            hasResults = true;
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
            if (!res->isNull("transaction_id")) {
                std::string transactionId = formatTransactionId(res->getString("transaction_id"));
                centerText("| Transaction ID : " + transactionId + std::string(15, ' ') + "|");
            }
            centerText("+------------------------------------------+");
            std::cout << "\n";
        }
        
        if (!hasResults) {
            centerText("No quotations found.");
        }
        
        std::cout << "\nPress Enter to continue...";
        // Clear any remaining input in buffer
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        std::cin.get();
        
        delete res;
        delete pstmt;
        delete con;
}

// ===== Search Module =====
void searchModule() {
    while (true) {
        clearScreen();
        printMenuTitle("Search Module");
        printHeader("SEARCH OPTIONS");
        centerText("1. Search Users");
        centerText("2. Search Errands");
        centerText("3. Search Receipts");
        centerText("4. Search Runners");
        centerText("5. Search Quotations");
        centerText("0. Back to Admin Dashboard");
        printHeader("");
        std::cout << "\n";

        int choice = getMenuChoice(0, 5);
        if (choice == 0) break;

        if (choice == 1) {
            searchUsers();
        }
        else if (choice == 2) {
            searchErrands();
        }
        else if (choice == 3) {
            searchReceipts();
        }
        else if (choice == 4) {
            searchRunners();
        }
        else if (choice == 5) {
            searchQuotations();
        }
    }
}

// ===== Reporting Module =====
void reportingModule() {
    while (true) {
        clearScreen();
        printMenuTitle("Reporting Module");
        printHeader("REPORTING OPTIONS");
        centerText("1. Monthly Errands Chart");
        centerText("2. System Revenue");
        centerText("3. Top Runners by Income");
        centerText("4. Top Runners by Errands");
        centerText("5. All Runner Revenue");
        centerText("6. View All Receipts");
        centerText("7. Save Report Snapshot");
        centerText("0. Back to Admin Dashboard");
        printHeader("");
        std::cout << "\n";

        int choice = getMenuChoice(0, 7);
        if (choice == 0) break;

        if (choice == 1) {
            showMonthlyErrandsChart();
        }
        else if (choice == 2) {
            showSystemRevenue();
        }
        else if (choice == 3) {
            showTopRunnersByIncome();
        }
        else if (choice == 4) {
            showTopRunnersByErrands();
        }
        else if (choice == 5) {
            showAllRunnerRevenue();
        }
        else if (choice == 6) {
            viewAllReceipts();
        }
        else if (choice == 7) {
            saveReportSnapshot();
        }
    }
}

// ===== Admin menu =====
void admin_menu(const std::string& adminUsername) {
    while (true) {
        clearScreen();
        printMenuTitle("Admin Dashboard - Welcome " + adminUsername);
        printHeader("ADMIN MENU");
        centerText("1. View all users");
        centerText("2. View all errands");
        centerText("3. Update errand status");
        centerText("4. Assign errand to runner");
        centerText("5. Register runner manually");
        centerText("6. Reporting Module");
        centerText("7. Search Module");
        centerText("0. Logout");
        printHeader("");
        std::cout << "\n";

        int choice = getMenuChoice(0, 7);
        if (choice == 0) break;

        switch (choice) {
        case 1: viewAllUsers(); break;
        case 2: viewAllErrands(); break;
        case 3: updateErrandStatusAdmin(); break;
        case 4: assignErrandToRunner(); break;
        case 5: registerRunner(); break;
        case 6: reportingModule(); break;
        case 7: searchModule(); break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}
