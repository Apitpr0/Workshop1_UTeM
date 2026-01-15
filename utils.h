#pragma once
#define NOMINMAX // Prevent Windows.h from defining min/max macros
#include <iostream>
#include <limits>
#include <string>
#include <regex>
#include <vector>
#include <iomanip>
#include <sstream>
#include <windows.h> // untuk GetConsoleScreenBufferInfo

// ===== Color Functions =====
// Color codes for Windows console
enum ConsoleColor {
    COLOR_BLACK = 0,
    COLOR_DARK_BLUE = 1,
    COLOR_DARK_GREEN = 2,
    COLOR_DARK_CYAN = 3,
    COLOR_DARK_RED = 4,
    COLOR_DARK_MAGENTA = 5,
    COLOR_DARK_YELLOW = 6,
    COLOR_LIGHT_GRAY = 7,
    COLOR_DARK_GRAY = 8,
    COLOR_BLUE = 9,
    COLOR_GREEN = 10,
    COLOR_CYAN = 11,
    COLOR_RED = 12,
    COLOR_MAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15
};

// Set text color
inline void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Reset to default color (light gray on black)
inline void resetColor() {
    setColor(COLOR_LIGHT_GRAY);
}

// Print colored text
inline void printColored(const std::string& text, int color) {
    setColor(color);
    std::cout << text;
    resetColor();
}

// Get console width (default 80 if can't detect) - moved up for use in centerColoredText
inline int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        // Use dwSize.X for actual console buffer width, or window size if available
        int width = csbi.dwSize.X;
        // If window is smaller than buffer, use window size
        int windowWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        if (windowWidth > 0 && windowWidth < width) {
            width = windowWidth;
        }
        return width;
    }
    return 80; // Default width
}

// Print centered colored text
inline void centerColoredText(const std::string& text, int color, int width = -1) {
    if (width == -1) width = getConsoleWidth();
    int padding = (width - static_cast<int>(text.length())) / 2;
    if (padding > 0) std::cout << std::string(padding, ' ');
    printColored(text, color);
    std::cout << "\n";
}

// Separator for table
inline void printSeparator(int width) {
    std::cout << std::string(width, '-') << "\n";
}

// ===== Validation functions =====
inline bool isValidEmail(const std::string& email) {
    // Basic regex pattern
    const std::regex pattern(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    if (!std::regex_match(email, pattern)) {
        return false;
    }
    
    // Find @ position
    size_t atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0) {
        return false;
    }
    
    // Check local part (before @)
    std::string localPart = email.substr(0, atPos);
    // Cannot start or end with dot
    if (localPart[0] == '.' || localPart[localPart.length() - 1] == '.') {
        return false;
    }
    // Cannot have consecutive dots
    if (localPart.find("..") != std::string::npos) {
        return false;
    }
    
    // Check domain part (after @)
    std::string domainPart = email.substr(atPos + 1);
    // Cannot start or end with dot or hyphen
    if (domainPart[0] == '.' || domainPart[domainPart.length() - 1] == '.' ||
        domainPart[0] == '-' || domainPart[domainPart.length() - 1] == '-') {
        return false;
    }
    // Cannot have consecutive dots
    if (domainPart.find("..") != std::string::npos) {
        return false;
    }
    
    return true;
}

inline bool isValidName(const std::string& u) {
    const std::regex pattern(R"(^[A-Za-z0-9]{2,50}$)"); // 2-50 chars, letters & numbers
    return std::regex_match(u, pattern);
}

inline bool isValidPassword(const std::string& pw) {
    if (pw.length() < 8) return false;
    if (!std::regex_search(pw, std::regex("[A-Z]"))) return false; // Uppercase
    if (!std::regex_search(pw, std::regex("[a-z]"))) return false; // Lowercase
    if (!std::regex_search(pw, std::regex("[0-9]"))) return false; // Digit
    if (!std::regex_search(pw, std::regex(R"([\W_])"))) return false; // Symbol
    std::vector<std::string> blacklist = { "password","12345678","qwerty","admin" };
    for (auto& s : blacklist) if (pw == s) return false;
    return true;
}

// ===== International phone validation =====
inline bool isValidPhoneIntl(const std::string& phone) {
    if (phone.empty()) return false;
    if (phone[0] == '+') {
        for (size_t i = 1; i < phone.size(); ++i)
            if (!std::isdigit(phone[i])) return false;
    }
    else {
        for (char c : phone) if (!std::isdigit(c)) return false;
    }
    size_t digitsCount = (phone[0] == '+') ? phone.size() - 1 : phone.size();
    return digitsCount >= 7 && digitsCount <= 15;
}

// Normalize phone for duplicate check
inline std::string normalizePhone(const std::string& input) {
    std::string num;
    for (char c : input) if (std::isdigit(c)) num += c;
    return num;
}

// Helper untuk truncate string panjang
inline std::string truncateString(const std::string& str, size_t width) {
    if (str.length() <= width) return str;
    if (width <= 3) return str.substr(0, width);
    return str.substr(0, width - 3) + "...";
}

// ===== Beautiful UI Functions =====
// Center text on screen
inline void centerText(const std::string& text, int width = -1) {
    if (width == -1) width = getConsoleWidth();
    int padding = (width - static_cast<int>(text.length())) / 2;
    if (padding > 0) std::cout << std::string(padding, ' ');
    std::cout << text;
    std::cout << "\n";
}

// Set cursor position
inline void setCursorPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = static_cast<SHORT>(x);
    coord.Y = static_cast<SHORT>(y);
    SetConsoleCursorPosition(hConsole, coord);
}

// Get cursor position
inline COORD getCursorPosition() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return csbi.dwCursorPosition;
}

// Print centered input prompt and position cursor for input
inline void centerInputPrompt(const std::string& prompt, int width = -1) {
    if (width == -1) width = getConsoleWidth();
    int padding = (width - static_cast<int>(prompt.length())) / 2;
    if (padding > 0) std::cout << std::string(padding, ' ');
    std::cout << prompt;
    std::cout.flush(); // Ensure prompt is displayed before positioning cursor
}

// Get centered input (string)
inline std::string getCenteredInput(const std::string& prompt, int width = -1) {
    if (width == -1) width = getConsoleWidth();
    centerInputPrompt(prompt, width);
    std::string input;
    std::getline(std::cin, input);
    return input;
}

// Get centered input (integer) - for numeric inputs
inline int getCenteredIntInput(const std::string& prompt, int width = -1) {
    if (width == -1) width = getConsoleWidth();
    int value;
    while (true) {
        centerInputPrompt(prompt, width);
        std::string input;
        std::getline(std::cin, input);
        
        // Check if input is empty
        if (input.empty()) {
            centerText("Invalid input! Please enter a valid number.");
            continue;
        }
        
        // Check if input contains only digits (allow negative numbers)
        bool isValid = true;
        size_t startIdx = 0;
        if (input[0] == '-') {
            startIdx = 1;
            if (input.length() == 1) {
                isValid = false; // Just a minus sign
            }
        }
        
        // Check for + sign or other invalid characters
        if (input[0] == '+' || input.find_first_not_of("0123456789", startIdx) != std::string::npos) {
            isValid = false;
        }
        
        if (!isValid) {
            centerText("Invalid input! Please enter a valid number (digits only, no + sign).");
            continue;
        }
        
        // Convert to integer
        try {
            value = std::stoi(input);
            return value;
        }
        catch (const std::exception&) {
            centerText("Invalid input! Number is too large or invalid.");
            continue;
        }
    }
}

// Get centered input (char) - for single character inputs
inline char getCenteredCharInput(const std::string& prompt, int width = -1) {
    if (width == -1) width = getConsoleWidth();
    char value;
    while (true) {
        centerInputPrompt(prompt, width);
        if (std::cin >> value) {
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            return value;
        }
        else {
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            centerText("Invalid input! Please enter a valid character.");
        }
    }
}

// Print centered header with border
inline void printHeader(const std::string& title, char borderChar = '=', int width = -1, int borderColor = COLOR_CYAN, int titleColor = COLOR_WHITE) {
    int consoleWidth = getConsoleWidth();
    if (width == -1) {
        // Use console width minus 4 for padding on both sides
        width = consoleWidth - 4;
        // But ensure minimum width based on title
        if (!title.empty()) {
            int titleLen = static_cast<int>(title.length());
            int minWidth = titleLen + 6;
            if (width < minWidth) {
                width = std::min(minWidth, consoleWidth - 2);
            }
        }
        // Ensure minimum reasonable width
        if (width < 40) width = 40;
    }
    std::string border(width, borderChar);
    centerColoredText(border, borderColor, consoleWidth);
    if (!title.empty()) {
        centerColoredText(title, titleColor, consoleWidth);
    }
    centerColoredText(border, borderColor, consoleWidth);
    std::cout << "\n";
}

// Print centered menu title
inline void printMenuTitle(const std::string& title, int width = -1) {
    std::cout << "\n";
    printHeader(title, '=', width, COLOR_CYAN, COLOR_YELLOW);
}

// Print beautiful separator
inline void printSeparator(char char1 = '-', char char2 = '-', int width = -1) {
    int consoleWidth = getConsoleWidth();
    if (width == -1) {
        width = consoleWidth - 4; // Leave padding on both sides
        if (width < 40) width = 40;
    }
    std::string sep(width, char1);
    centerText(sep, consoleWidth);
}

// Print centered message in box
inline void printBoxedMessage(const std::string& message, int width = -1, int borderColor = COLOR_CYAN) {
    int consoleWidth = getConsoleWidth();
    if (width == -1) width = consoleWidth;
    int boxWidth = std::min(static_cast<int>(message.length()) + 4, width - 4);
    if (boxWidth < static_cast<int>(message.length()) + 4) boxWidth = static_cast<int>(message.length()) + 4;
    std::string topBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    std::string bottomBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    
    centerColoredText(topBorder, borderColor, consoleWidth);
    int msgPadding = boxWidth - static_cast<int>(message.length()) - 2;
    std::string msgLine = "| " + message + std::string(msgPadding > 0 ? msgPadding : 0, ' ') + "|";
    centerText(msgLine, consoleWidth);
    centerColoredText(bottomBorder, borderColor, consoleWidth);
    std::cout << "\n";
}

// Print table header with beautiful borders
inline void printTableHeader(const std::vector<std::pair<std::string, int>>& columns, int width = -1, int borderColor = COLOR_CYAN, int headerColor = COLOR_YELLOW) {
    if (width == -1) width = getConsoleWidth();
    int totalWidth = 0;
    for (const auto& col : columns) {
        totalWidth += col.second;
    }
    totalWidth += static_cast<int>(columns.size()) + 1; // borders
    
    int padding = (width - totalWidth) / 2;
    if (padding > 0) std::cout << std::string(padding, ' ');
    
    // Top border
    setColor(borderColor);
    std::cout << "+";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::cout << std::string(columns[i].second, '-');
        if (i < columns.size() - 1) std::cout << "+";
    }
    std::cout << "+\n";
    resetColor();
    
    // Header row
    if (padding > 0) std::cout << std::string(padding, ' ');
    setColor(borderColor);
    std::cout << "|";
    resetColor();
    for (const auto& col : columns) {
        setColor(headerColor);
        std::cout << std::left << std::setw(col.second) << col.first;
        resetColor();
        setColor(borderColor);
        std::cout << "|";
        resetColor();
    }
    std::cout << "\n";
    
    // Separator
    if (padding > 0) std::cout << std::string(padding, ' ');
    setColor(borderColor);
    std::cout << "+";
    for (size_t i = 0; i < columns.size(); ++i) {
        std::cout << std::string(columns[i].second, '-');
        if (i < columns.size() - 1) std::cout << "+";
    }
    std::cout << "+\n";
    resetColor();
}

// Print table row
inline void printTableRow(const std::vector<std::string>& values, const std::vector<int>& widths, int width = -1, int borderColor = COLOR_CYAN) {
    if (width == -1) width = getConsoleWidth();
    int totalWidth = 0;
    for (int w : widths) {
        totalWidth += w;
    }
    totalWidth += static_cast<int>(widths.size()) + 1;
    
    int padding = (width - totalWidth) / 2;
    if (padding > 0) std::cout << std::string(padding, ' ');
    
    setColor(borderColor);
    std::cout << "|";
    resetColor();
    for (size_t i = 0; i < values.size() && i < widths.size(); ++i) {
        std::string val = values[i];
        if (val.length() > static_cast<size_t>(widths[i])) {
            val = val.substr(0, widths[i] - 3) + "...";
        }
        std::cout << std::left << std::setw(widths[i]) << val;
        setColor(borderColor);
        std::cout << "|";
        resetColor();
    }
    std::cout << "\n";
}

// Print table footer
inline void printTableFooter(const std::vector<int>& widths, int width = -1, int borderColor = COLOR_CYAN) {
    if (width == -1) width = getConsoleWidth();
    int totalWidth = 0;
    for (int w : widths) {
        totalWidth += w;
    }
    totalWidth += static_cast<int>(widths.size()) + 1;
    
    int padding = (width - totalWidth) / 2;
    if (padding > 0) std::cout << std::string(padding, ' ');
    
    setColor(borderColor);
    std::cout << "+";
    for (size_t i = 0; i < widths.size(); ++i) {
        std::cout << std::string(widths[i], '-');
        if (i < widths.size() - 1) std::cout << "+";
    }
    std::cout << "+\n";
    resetColor();
}

// Clear screen
inline void clearScreen() {
    system("cls");
}

// Print success message
inline void printSuccess(const std::string& message) {
    int consoleWidth = getConsoleWidth();
    int boxWidth = std::min(static_cast<int>(message.length()) + 20, consoleWidth - 4);
    if (boxWidth < static_cast<int>(message.length()) + 20) boxWidth = static_cast<int>(message.length()) + 20;
    std::string topBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    std::string bottomBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    
    centerColoredText(topBorder, COLOR_GREEN, consoleWidth);
    int msgPadding = boxWidth - static_cast<int>(message.length()) - 18;
    std::string msgLine = "| ";
    setColor(COLOR_GREEN);
    msgLine += "[SUCCESS]";
    resetColor();
    msgLine += " " + message + std::string(msgPadding > 0 ? msgPadding : 0, ' ') + "|";
    centerText(msgLine, consoleWidth);
    centerColoredText(bottomBorder, COLOR_GREEN, consoleWidth);
    std::cout << "\n";
}

// Print error message
inline void printError(const std::string& message) {
    int consoleWidth = getConsoleWidth();
    int boxWidth = std::min(static_cast<int>(message.length()) + 17, consoleWidth - 4);
    if (boxWidth < static_cast<int>(message.length()) + 17) boxWidth = static_cast<int>(message.length()) + 17;
    std::string topBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    std::string bottomBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    
    centerColoredText(topBorder, COLOR_RED, consoleWidth);
    int msgPadding = boxWidth - static_cast<int>(message.length()) - 15;
    std::string msgLine = "| ";
    setColor(COLOR_RED);
    msgLine += "[ERROR]";
    resetColor();
    msgLine += " " + message + std::string(msgPadding > 0 ? msgPadding : 0, ' ') + "|";
    centerText(msgLine, consoleWidth);
    centerColoredText(bottomBorder, COLOR_RED, consoleWidth);
    std::cout << "\n";
}

// Print info message
inline void printInfo(const std::string& message) {
    int consoleWidth = getConsoleWidth();
    int boxWidth = std::min(static_cast<int>(message.length()) + 14, consoleWidth - 4);
    if (boxWidth < static_cast<int>(message.length()) + 14) boxWidth = static_cast<int>(message.length()) + 14;
    std::string topBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    std::string bottomBorder = "+" + std::string(boxWidth - 2, '-') + "+";
    
    centerColoredText(topBorder, COLOR_CYAN, consoleWidth);
    int msgPadding = boxWidth - static_cast<int>(message.length()) - 12;
    std::string msgLine = "| ";
    setColor(COLOR_CYAN);
    msgLine += "[INFO]";
    resetColor();
    msgLine += " " + message + std::string(msgPadding > 0 ? msgPadding : 0, ' ') + "|";
    centerText(msgLine, consoleWidth);
    centerColoredText(bottomBorder, COLOR_CYAN, consoleWidth);
    std::cout << "\n";
}

// Inline supaya boleh include di banyak file tanpa duplicate
// Note: This function must be defined after centerInputPrompt and printError
inline int getMenuChoice(int min, int max) {
    int choice;
    while (true) {
        centerInputPrompt("Enter choice: ");
        std::string input;
        std::getline(std::cin, input);
        
        // Check if input is empty
        if (input.empty()) {
            printError("Invalid input. Enter a number between " + std::to_string(min) + " and " + std::to_string(max) + ".");
            continue;
        }
        
        // Check if input contains only digits (no +, -, or other characters)
        if (input.find_first_not_of("0123456789") != std::string::npos) {
            printError("Invalid input. Enter a number between " + std::to_string(min) + " and " + std::to_string(max) + " (digits only, no + or - sign).");
            continue;
        }
        
        // Convert to integer
        try {
            choice = std::stoi(input);
            if (choice >= min && choice <= max) {
                return choice;
            }
            else {
                printError("Invalid input. Enter a number between " + std::to_string(min) + " and " + std::to_string(max) + ".");
            }
        }
        catch (const std::exception&) {
            printError("Invalid input. Enter a number between " + std::to_string(min) + " and " + std::to_string(max) + ".");
        }
    }
}
