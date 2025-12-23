// dashboard.cpp
#include "dashboard.h"
#include "user_dashboard.h"
#include "admin_dashboard.h"
#include "runner_dashboard.h"
#include "loginpage.h"
#include <iostream>
#include <limits>
#include <tuple>
#include <string>

// ===== Menu helper =====
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

// ===== Main dashboard =====
void show_dashboard() {
    while (true) {
        std::cout << "\n=== ERMS Main Dashboard ===\n";
        std::cout << "1. User Login\n2. Admin Login\n3. Runner Login\n4. Register\n0. Exit\nEnter choice: ";
        int choice = getMenuChoice(0, 4);

        switch (choice) {
        case 0:
            return;

        case 1: {
            std::tuple<int, std::string> loginResult = user_login();
            int role = std::get<0>(loginResult);
            std::string username = std::get<1>(loginResult);
            if (role == 0) {

                std::cout << "[User] Login successful! Welcome " << username << ".\n";
                user_menu(username);
            }
            else {
                std::cout << "Login failed or wrong role.\n";
            }
            break;
        }

        case 2: {
            std::tuple<int, std::string> loginResult = admin_login();
            int role = std::get<0>(loginResult);
            std::string username = std::get<1>(loginResult);
            if (role == 1) {
                std::cout << "[Admin] Login successful! Welcome " << username << ".\n";
                admin_menu(username); 
            }
            else {
                std::cout << "Login failed or wrong role.\n";
            }
            break;
        }

        case 3: {
            std::tuple<int, std::string> loginResult = runner_login();
            int role = std::get<0>(loginResult);
            std::string username = std::get<1>(loginResult);

            if (role == 2) {
                std::cout << "[Runner] Login successful! Welcome " << username << ".\n";
                runner_menu(username);  // <-- Panggil dashboard baru di sini
            }
            else {
                std::cout << "Login failed or wrong role.\n";
            }
            break;
        }


        case 4:
            registerUser();
            break;

        default:
            std::cout << "Invalid choice.\n";
        }
    }
}
