#include <iostream>
#include <string>

bool login(const std::string& username, const std::string& password) {
    // Hardcoded credentials for demonstration
    const std::string valid_username = "admin";
    const std::string valid_password = "password123";

    return (username == valid_username && password == valid_password);
}

void show_login_page() {
    std::string username, password;

    std::cout << "=== Login Page ===" << std::endl;
    std::cout << "Username: ";
    std::getline(std::cin, username);
    std::cout << "Password: ";
    std::getline(std::cin, password);

    if (login(username, password)) {
        std::cout << "Login successful! Welcome, " << username << "." << std::endl;
    } else {
        std::cout << "Login failed! Invalid username or password." << std::endl;
    }
}