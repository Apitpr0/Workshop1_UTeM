// dashboard.cpp
#include <iostream>
#include <limits>

// Forward declarations
bool registerUser();
int show_login_page();

// Admin menu
void admin_menu() {
    while (true) {
        std::cout << "\n=== Admin Dashboard ===\n";
        std::cout << "1. View all users\n2. Manage errands\n0. Logout\nEnter choice: ";
        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (choice == 0) break;
        switch (choice) {
        case 1: std::cout << "[Admin] Viewing all users...\n"; break;
        case 2: std::cout << "[Admin] Managing errands...\n"; break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}

// User menu
void user_menu() {
    while (true) {
        std::cout << "\n=== User Dashboard ===\n";
        std::cout << "1. View my errands\n2. Create new errand\n0. Logout\nEnter choice: ";
        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (choice == 0) break;
        switch (choice) {
        case 1: std::cout << "[User] Viewing your errands...\n"; break;
        case 2: std::cout << "[User] Creating a new errand...\n"; break;
        default: std::cout << "Invalid choice.\n";
        }
    }
}

// Main dashboard
void show_dashboard() {
    while (true) {
        std::cout << "\n=== Main Dashboard ===\n";
        std::cout << "1. Login\n2. Register\n0. Exit\nEnter choice: ";
        int choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if (choice == 0) break;

        switch (choice) {
        case 1: {
            int role = show_login_page();
            if (role == 1) admin_menu();
            else if (role == 0) user_menu();
            break;
        }
        case 2:
            registerUser();
            break;
        default:
            std::cout << "Invalid choice.\n";
        }
    }
}
