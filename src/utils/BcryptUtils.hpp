#pragma once

#include <string>
#include <vector>

namespace utils {

class BcryptUtils {
public:
    // Generate a random salt
    static std::string generateSalt(int rounds = 12);

    // Hash a password using bcrypt
    static std::string hashPassword(const std::string& password, const std::string& salt);

    // Verify a password against a hashed password
    static bool verifyPassword(const std::string& password, const std::string& hashed_password);

private:
    // Base64 encode a byte array
    static std::string base64Encode(const std::vector<unsigned char>& data);

    // Base64 decode a string
    static std::vector<unsigned char> base64Decode(const std::string& data);

    // Blowfish key expansion
    static void blowfishExpandKey(const std::vector<unsigned char>& key, const std::vector<unsigned char>& salt, std::vector<unsigned int>& p, std::vector<unsigned int>& s);

    // Blowfish encryption
    static void blowfishEncrypt(unsigned int* xl, unsigned int* xr, const std::vector<unsigned int>& p, const std::vector<unsigned int>& s);

    // BCrypt hash function
    static std::vector<unsigned char> bcryptHash(const std::vector<unsigned char>& password, const std::vector<unsigned char>& salt, int rounds);
};

} // namespace utils