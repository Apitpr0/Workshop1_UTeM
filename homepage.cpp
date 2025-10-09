#include <iostream>
#include <string>

// Forward declaration of the login page function
void show_login_page();

void show_dashboard() {
    int choice;

    while (true) {
        std::cout << "=== Dashboard ===" << std::endl;
        std::cout << "1. Go to Login Page" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer

        if (choice == 1) {
            show_login_page();
        } else if (choice == 0) {
            std::cout << "Exiting dashboard." << std::endl;
            break;
        } else {
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}