#pragma once

#include <string>
#include <map>
#include <chrono>

namespace utils {

class AuthUtils {
public:
    // Generate a new access token for a user
    static std::string generateToken(int user_id, const std::string& username);

    // Validate an access token and extract user information
    static bool validateToken(const std::string& token, int& user_id, std::string& username);

    // Verify a token and get the user ID
    static int verifyTokenAndGetUserId(const std::string& token);

    // Hash a password using bcrypt
    static std::string hashPassword(const std::string& password);

    // Verify a password against a hashed password
    static bool verifyPassword(const std::string& password, const std::string& hashed_password);

    // Validate email format
    static bool validateEmail(const std::string& email);

    // Validate username format (alphanumeric, 3-20 characters)
    static bool validateUsername(const std::string& username);

    // Validate password strength (at least 8 characters, contains letters and numbers)
    static bool validatePasswordStrength(const std::string& password);

private:
    // Secret key used for token generation and validation
    static const std::string secret_key_;

    // Token expiration time (in hours)
    static const int token_expiration_hours_;

    // Helper function to create a HMAC-SHA256 signature
    static std::string createHmac(const std::string& data);

    // Helper function to parse a token into its components
    static bool parseToken(const std::string& token, std::string& header, std::string& payload, std::string& signature);

    // Helper function to decode a base64url encoded string
    static std::string base64UrlDecode(const std::string& encoded_str);

    // Helper function to encode a string into base64url format
    static std::string base64UrlEncode(const std::string& str);
};

} // namespace utils
