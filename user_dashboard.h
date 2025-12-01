#pragma once
#include <string>

// Returns the user ID for a given username, or -1 if not found
int getUserId(const std::string& username);

// Displays all errands for the given user
void viewMyErrands(const std::string& username);

// Creates a new errand for the given user
void createNewErrand(const std::string& username);

// User menu loop
void user_menu(const std::string& username);
