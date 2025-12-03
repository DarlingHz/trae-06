#include "auth.h"
#include <sstream>
#include <openssl/sha.h>
#include <iomanip>
#include <ctime>
#include <algorithm>

std::string AuthManager::hash_password(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.length(), hash);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    
    return ss.str();
}

bool AuthManager::verify_password(const std::string& password, const std::string& hash) {
    return hash_password(password) == hash;
}

std::string AuthManager::generate_token(const UserDTO& user) {
    std::stringstream ss;
    ss << user.id << "|" << user.email << "|" << time(nullptr) << "|" << user.role;
    return ss.str();
}

std::optional<UserDTO> AuthManager::verify_token(const std::string& token) {
    std::stringstream ss(token);
    std::string part;
    std::vector<std::string> parts;
    
    while (std::getline(ss, part, '|')) {
        parts.push_back(part);
    }
    
    if (parts.size() != 4) {
        return std::nullopt;
    }
    
    UserDTO user;
    try {
        user.id = std::stoi(parts[0]);
        user.email = parts[1];
        user.role = parts[3];
    } catch (...) {
        return std::nullopt;
    }
    
    return user;
}

std::optional<std::string> AuthManager::extract_token_from_header(const std::string& header) {
    const std::string prefix = "Bearer ";
    if (header.size() > prefix.size() && header.substr(0, prefix.size()) == prefix) {
        return header.substr(prefix.size());
    }
    return std::nullopt;
}