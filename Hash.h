#pragma once
#include <string>

std::string hashPassword(const std::string& password);
bool verifyPassword(const std::string& input, const std::string& storedHash);
