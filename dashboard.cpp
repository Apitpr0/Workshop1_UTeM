// dashboard.cpp
#include <iostream>
#include <limits>
#include <tuple>
#include <string>
#include "user_dashboard.h"  // include user dashboard functions

// Forward declarations
bool registerUser();
std::tuple<int, std::string> show_login_page(); // returns {role, username}

// Helper function for safe integer input
int getMenuChoice(int min, int max) {
    int choice;
    while (true) {
        std::cin >> choice;
        if (!std::cin.fail() && choice >= min && choice <= max) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Enter a number between " << min << " and " << max << ": ";
    }
}

// ===== Admin menu =====
void admin_menu() {
    while (true) {
        std::cout << "\n=== Admin Dashboard ===\n";
        std::cout << "1. View all users\n2. Manage errands\n0. Logout\nEnter choice: ";
        int choice = getMenuChoice(0, 2);

        if (choice == 0) break;
        switch (choice) {
        case 1: std::cout << "[Admin] Viewing all users (to be implemented)...\n"; break;
        case 2: std::cout << "[Admin] Managing errands (to be implemented)...\n"; break;
        }
    }
}

// ===== Main dashboard =====
void show_dashboard() {
    while (true) {
        std::cout << "\n=== Main Dashboard ===\n";
        std::cout << "1. Login\n2. Register\n0. Exit\nEnter choice: ";
        int choice = getMenuChoice(0, 2);

        switch (choice) {
        case 0:
            return;
        case 1: {
            // Pre-C++17 way to unpack tuple
            std::tuple<int, std::string> loginResult = show_login_page();
            int role = std::get<0>(loginResult);
            std::string username = std::get<1>(loginResult);

            if (role == 1) admin_menu();
            else if (role == 0) user_menu(username); // now calls MySQL-integrated user menu
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
