#include "utils/hash.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <cstring>

Hash::Hash() {}

Hash::~Hash() {}

Hash& Hash::getInstance() {
    static Hash instance;
    return instance;
}

std::string Hash::generateSalt() {
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Failed to generate salt");
    }

    return std::string(reinterpret_cast<char*>(salt), sizeof(salt));
}

std::string Hash::hashPassword(const std::string& password, const std::string& salt) {
    const int iterations = 100000;
    const int key_length = 64;
    unsigned char hash[key_length];

    if (PKCS5_PBKDF2_HMAC_SHA256(
            password.c_str(), password.length(),
            reinterpret_cast<const unsigned char*>(salt.c_str()), salt.length(),
            iterations, key_length, hash) != 1) {
        throw std::runtime_error("Failed to hash password");
    }

    return std::string(reinterpret_cast<char*>(hash), sizeof(hash));
}

bool Hash::verifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    std::string new_hash = hashPassword(password, salt);
    return new_hash == hash;
}
