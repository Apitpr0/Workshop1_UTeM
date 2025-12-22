#include "hash.h"
#include <string>
#include <sstream>
#include <functional>

static const std::string SALT = "ERMS_WORKSHOP1_SALT_2025";
static const int HASH_ROUNDS = 1000;

std::string hashPassword(const std::string& password) {
    std::string data = password + SALT;
    std::hash<std::string> hasher;

    size_t hash = hasher(data);
    for (int i = 0; i < HASH_ROUNDS; ++i) {
        hash = hasher(std::to_string(hash) + SALT);
    }

    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

bool verifyPassword(const std::string& input, const std::string& storedHash) {
    return hashPassword(input) == storedHash;
}
