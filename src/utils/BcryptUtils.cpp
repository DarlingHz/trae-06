#include "utils/BcryptUtils.hpp"
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace utils {

// Base64 encoding table
const char base64_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

// Base64 decoding table
const int base64_decode_table[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 62,
    -1, -1, -1, 63, 52, 53, 54, 55, 56, 57,
    58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 23, 24, 25, -1, -1, -1,
    -1, -1, -1, 26, 27, 28, 29, 30, 31, 32,
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, -1,
    -1, -1, -1, -1
};

// Generate a random salt
std::string BcryptUtils::generateSalt(int rounds) {
    if (rounds < 4 || rounds > 31) {
        throw std::invalid_argument("Invalid number of rounds (must be between 4 and 31)");
    }

    // Generate 16 random bytes
    std::vector<unsigned char> salt_bytes(16);
    srand(time(nullptr));
    for (int i = 0; i < 16; i++) {
        salt_bytes[i] = rand() % 256;
    }

    // Base64 encode the salt
    std::string salt = base64Encode(salt_bytes);

    // Truncate to 22 characters (bcrypt uses 22 base64 characters for the salt)
    if (salt.length() > 22) {
        salt = salt.substr(0, 22);
    }

    // Return the salt in the format $2a$XX$XXXXXXXXXXXXXXXXXXXXXXXXXX
    return "$2a$" + std::to_string(rounds) + "$" + salt;
}

// Hash a password using bcrypt
std::string BcryptUtils::hashPassword(const std::string& password, const std::string& salt) {
    // Parse the salt
    int rounds = 0;
    std::string salt_bytes_str;

    // Check if the salt is in the correct format
    if (salt.length() < 29 || salt.substr(0, 7) != "$2a$XX$") {
        throw std::invalid_argument("Invalid salt format");
    }

    // Extract the number of rounds
    try {
        rounds = std::stoi(salt.substr(4, 2));
    } catch (const std::exception& e) {
        throw std::invalid_argument("Invalid number of rounds in salt");
    }

    // Extract the salt bytes
    salt_bytes_str = salt.substr(7, 22);

    // Base64 decode the salt bytes
    std::vector<unsigned char> salt_bytes = base64Decode(salt_bytes_str);

    // Convert the password to a vector of unsigned char
    std::vector<unsigned char> password_bytes(password.begin(), password.end());

    // Hash the password
    std::vector<unsigned char> hash_bytes = bcryptHash(password_bytes, salt_bytes, rounds);

    // Base64 encode the hash bytes
    std::string hash = base64Encode(hash_bytes);

    // Truncate to 31 characters (bcrypt uses 31 base64 characters for the hash)
    if (hash.length() > 31) {
        hash = hash.substr(0, 31);
    }

    // Return the hashed password in the format $2a$XX$XXXXXXXXXXXXXXXXXXXXXXXXXXYYYYYYYYYYYYYYYYYYYYYYYYYYY
    return "$2a$" + std::to_string(rounds) + "$" + salt_bytes_str + hash;
}

// Verify a password against a hashed password
bool BcryptUtils::verifyPassword(const std::string& password, const std::string& hashed_password) {
    // Check if the hashed password is in the correct format
    if (hashed_password.length() < 60 || hashed_password.substr(0, 7) != "$2a$XX$") {
        return false;
    }

    // Extract the salt from the hashed password
    std::string salt = hashed_password.substr(0, 29);

    // Hash the password using the extracted salt
    std::string hashed_password_2;
    try {
        hashed_password_2 = hashPassword(password, salt);
    } catch (const std::exception& e) {
        return false;
    }

    // Compare the two hashed passwords
    return hashed_password == hashed_password_2;
}

// Base64 encode a byte array
std::string BcryptUtils::base64Encode(const std::vector<unsigned char>& data) {
    std::string encoded;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    int length = data.size();
    const unsigned char* bytes_to_encode = data.data();

    while (length--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                encoded += base64_table[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; j < i + 1; j++) {
            encoded += base64_table[char_array_4[j]];
        }

        while (i++ < 3) {
            encoded += '=';
        }
    }

    return encoded;
}

// Base64 decode a string
std::vector<unsigned char> BcryptUtils::base64Decode(const std::string& data) {
    std::vector<unsigned char> decoded;
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4];
    unsigned char char_array_3[3];

    int length = data.size();
    const char* bytes_to_decode = data.c_str();

    while (length-- && (bytes_to_decode[in_] != '=') && base64_decode_table[static_cast<unsigned char>(bytes_to_decode[in_])] != -1) {
        char_array_4[i++] = bytes_to_decode[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_decode_table[char_array_4[i]];
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
                decoded.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            char_array_4[j] = base64_decode_table[char_array_4[j]];
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; j < i - 1; j++) {
            decoded.push_back(char_array_3[j]);
        }
    }

    return decoded;
}

// Blowfish key expansion
void BcryptUtils::blowfishExpandKey(const std::vector<unsigned char>& key, const std::vector<unsigned char>& salt, std::vector<unsigned int>& p, std::vector<unsigned int>& s) {
    // Initialize p and s with the Blowfish P-array and S-boxes
    // (These are the standard Blowfish P-array and S-boxes)
    unsigned int p_init[] = {
        0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
        0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
        0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
        0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917,
        0x9216d5d9, 0x8979fb1b
    };

    unsigned int s_init[4][256] = {
        {0xd1310ba6, 0x98dfb5ac, 0x2ffd72db, 0xd01adfb7, 0xb8e1afed, 0x6a267e96, 0xba7c9045, 0xf12c7f99, 0x24a19947, 0xb3916cf7, 0x0801f2e2, 0x858efc16, 0x636920d8, 0x71574e69, 0xa458fea3},
        {0xf4933d7e, 0x0d95748f, 0x728eb658, 0x718bcd58, 0x82154aee, 0x7b54a41d, 0xc25a59b5, 0x9c30d539, 0x2af26013, 0xc5d1b023, 0x286085f0, 0xca417918, 0xb8db38ef, 0x8e79dcb0, 0x603a180e},
        {0x6c9e0e8b, 0xb01e8a3e, 0xd71577c1, 0xbd314b27, 0x78af2fda, 0x55605c60, 0xe65525f3, 0xaa55ab94, 0x57489862, 0x63e81440, 0x55ca396a, 0x2aab10b6, 0xb4cc5c34, 0x1141e8ce, 0xa15486af},
        {0x7c72e993, 0xb3ee1411, 0x636fbc2a, 0x2ba9c55d, 0x741831f6, 0xce5c3e16, 0x9b87931e, 0xafd6ba33, 0x6c24cf5c, 0x7a325381, 0x28958677, 0x3b8f4898, 0x6b4bb9af, 0xc4bfe81b, 0x66282193}
    };

    // Copy the initial P-array and S-boxes to p and s
    p.assign(p_init, p_init + sizeof(p_init) / sizeof(unsigned int));
    s.resize(4 * 256);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 256; j++) {
            s[i * 256 + j] = s_init[i][j];
        }
    }

    // XOR the P-array with the key
    int key_length = key.size();
    for (int i = 0; i < 18; i++) {
        unsigned int key_byte = 0;
        for (int j = 0; j < 4; j++) {
            key_byte = (key_byte << 8) | key[(i * 4 + j) % key_length];
        }
        p[i] ^= key_byte;
    }

    // XOR the P-array with the salt
    int salt_length = salt.size();
    for (int i = 0; i < 18; i++) {
        unsigned int salt_byte = 0;
        for (int j = 0; j < 4; j++) {
            salt_byte = (salt_byte << 8) | salt[(i * 4 + j) % salt_length];
        }
        p[i] ^= salt_byte;
    }

    // Update the P-array and S-boxes using the Blowfish encryption function
    unsigned int xl = 0;
    unsigned int xr = 0;
    for (int i = 0; i < 18; i += 2) {
        blowfishEncrypt(&xl, &xr, p, s);
        p[i] = xl;
        p[i + 1] = xr;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 256; j += 2) {
            blowfishEncrypt(&xl, &xr, p, s);
            s[i * 256 + j] = xl;
            s[i * 256 + j + 1] = xr;
        }
    }
}

// Blowfish encryption
void BcryptUtils::blowfishEncrypt(unsigned int* xl, unsigned int* xr, const std::vector<unsigned int>& p, const std::vector<unsigned int>& s) {
    unsigned int xL = *xl;
    unsigned int xR = *xr;

    // Initial XOR with P-array
    xL ^= p[0];
    xR ^= p[1];

    // 16 rounds of Blowfish
    for (int i = 1; i <= 16; i++) {
        unsigned int temp = xL;
        xL = xR ^ (s[(temp >> 24) & 0xff] + s[((temp >> 16) & 0xff) + 256] ^ s[((temp >> 8) & 0xff) + 512] + s[(temp & 0xff) + 768]);
        xR = temp;
        xL ^= p[i + 1];
        xR ^= p[i + 2];
    }

    // Final swap and XOR with P-array
    unsigned int temp = xL;
    xL = xR;
    xR = temp;
    xL ^= p[17];
    xR ^= p[18];

    // Return the encrypted values
    *xl = xL;
    *xr = xR;
}

// BCrypt hash function
std::vector<unsigned char> BcryptUtils::bcryptHash(const std::vector<unsigned char>& password, const std::vector<unsigned char>& salt, int rounds) {
    // Initialize the Blowfish P-array and S-boxes
    std::vector<unsigned int> p;
    std::vector<unsigned int> s;
    blowfishExpandKey(password, salt, p, s);

    // Perform the key expansion multiple times (based on the number of rounds)
    for (int i = 0; i < (1 << rounds); i++) {
        // Step 3a: Expand key using password and current P-array
        std::vector<unsigned char> p_bytes_a(p.size() * sizeof(unsigned int));
        memcpy(p_bytes_a.data(), p.data(), p_bytes_a.size());
        blowfishExpandKey(password, p_bytes_a, p, s);

        // Step 3b: Expand key using salt and current P-array (modified from step 3a)
        std::vector<unsigned char> p_bytes_b(p.size() * sizeof(unsigned int));
        memcpy(p_bytes_b.data(), p.data(), p_bytes_b.size());
        blowfishExpandKey(salt, p_bytes_b, p, s);
    }

    // Encrypt the text "OrpheanBeholderScryDoubt" 64 times
    unsigned char text[] = "OrpheanBeholderScryDoubt";
    unsigned int xl = 0;
    unsigned int xr = 0;

    for (int i = 0; i < 64; i++) {
        // Split the text into two 32-bit words
        xl = (text[0] << 24) | (text[1] << 16) | (text[2] << 8) | text[3];
        xr = (text[4] << 24) | (text[5] << 16) | (text[6] << 8) | text[7];

        // Encrypt the two words
        blowfishEncrypt(&xl, &xr, p, s);

        // Update the text with the encrypted words
        text[0] = (xl >> 24) & 0xff;
        text[1] = (xl >> 16) & 0xff;
        text[2] = (xl >> 8) & 0xff;
        text[3] = xl & 0xff;
        text[4] = (xr >> 24) & 0xff;
        text[5] = (xr >> 16) & 0xff;
        text[6] = (xr >> 8) & 0xff;
        text[7] = xr & 0xff;
    }

    // Return the encrypted text as a vector of unsigned char
    return std::vector<unsigned char>(text, text + sizeof(text));
}

} // namespace utils