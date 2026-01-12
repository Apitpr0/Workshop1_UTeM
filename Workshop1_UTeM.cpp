#include <iostream>

// Forward declarations
void test_db_connection();
void show_dashboard();

int main() {
    // Step 1: Test DB connection
    test_db_connection();

    // Step 2: Launch dashboard (handles login/register)
    show_dashboard();

    // Program ends here
    std::cout << "\nExiting program. Thank You for using ERMS!\n";
    return 0;
}
