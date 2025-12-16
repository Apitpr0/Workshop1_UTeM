#pragma once
#include <iostream>
#include <limits>
#include <string>

// Inline supaya boleh include di banyak file tanpa duplicate
inline int getMenuChoice(int min, int max) {
    int choice;
    while (true) {
        std::cin >> choice;
        if (std::cin.fail() || choice < min || choice > max) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Enter a number between " << min << " and " << max << ": ";
        }
        else {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return choice;
        }
    }
}

// Separator for table
inline void printSeparator(int width) {
    std::cout << std::string(width, '-') << "\n";
}
