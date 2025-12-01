#include "util/Utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <random>
#include <openssl/sha.h>
#include <openssl/rand.h>

namespace util {

namespace time {

bool parseIsoString(const std::string& iso_str, struct tm& tm_out) {
    std::istringstream ss(iso_str);
    ss >> std::get_time(&tm_out, "%Y-%m-%dT%H:%M:%S");
    return !ss.fail();
}

std::string toIsoString(const struct tm& tm) {
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    return ss.str();
}

double calculateHoursDiff(const struct tm& start_tm, const struct tm& end_tm) {
    time_t start_time = mktime(const_cast<struct tm*>(&start_tm));
    time_t end_time = mktime(const_cast<struct tm*>(&end_tm));
    double diff_seconds = difftime(end_time, start_time);
    return diff_seconds / 3600.0;
}

std::string extractDateFromIsoString(const std::string& iso_str) {
    size_t t_pos = iso_str.find('T');
    if (t_pos == std::string::npos) {
        return iso_str;
    }
    return iso_str.substr(0, t_pos);
}

} // namespace time

namespace json {

std::string toString(const ::json& json_obj) {
    return json_obj.dump();
}

::json fromString(const std::string& json_str) {
    try {
        return ::json::parse(json_str);
    } catch (const ::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return ::json::object();
    }
}

} // namespace json

namespace string {

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    tokens.push_back(str.substr(start));
    return tokens;
}

std::string trim(const std::string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        start++;
    }

    auto end = str.end();
    do {
        end--;
    } while (distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

std::string toLower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return lower_str;
}

std::string toUpper(const std::string& str) {
    std::string upper_str = str;
    std::transform(upper_str.begin(), upper_str.end(), upper_str.begin(),
        [](unsigned char c) { return std::toupper(c); });
    return upper_str;
}

} // namespace string

namespace crypto {

std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

std::string generateRandomString(int length) {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string random_str;
    random_str.reserve(length);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.size() - 1);

    for (int i = 0; i < length; i++) {
        random_str += charset[distribution(generator)];
    }

    return random_str;
}

} // namespace crypto

} // namespace util
