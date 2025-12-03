#include "job_service/utils.h"
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>
#include <stdexcept>

namespace job_service {
namespace utils {

std::string generate_job_id() {
    static std::random_device rd;
    static std::mt19937_64 rng(rd());
    static std::uniform_int_distribution<uint64_t> dist;
    
    auto now = std::chrono::system_clock::now();
    auto epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    auto random_num = dist(rng);
    
    std::ostringstream oss;
    oss << std::hex << epoch_ms << "_" << std::hex << random_num;
    return oss.str();
}

std::string time_to_iso_string(const std::chrono::system_clock::time_point& time) {
    auto time_t = std::chrono::system_clock::to_time_t(time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    oss << "." << std::setw(3) << std::setfill('0') << ms.count() << "Z";
    return oss.str();
}

std::optional<std::chrono::system_clock::time_point> iso_string_to_time(const std::string& str) {
    try {
        std::tm tm = {};
        std::istringstream ss(str);
        
        if (ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S")) {
            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            return tp;
        }
    } catch (...) {
        // 解析失败返回nullopt
    }
    
    return std::nullopt;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string trim(const std::string& str) {
    auto start = str.begin();
    while (start != str.end() && std::isspace(static_cast<unsigned char>(*start))) {
        start++;
    }
    
    auto end = str.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(static_cast<unsigned char>(*end)));
    
    return std::string(start, end + 1);
}

std::string to_lowercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string remove_punctuation(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (std::isalnum(static_cast<unsigned char>(c)) || std::isspace(static_cast<unsigned char>(c))) {
            result += c;
        }
    }
    return result;
}

std::string generate_random_string(size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 rng(rd());
    static std::uniform_int_distribution<> dist(0, sizeof(charset) - 2);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[dist(rng)];
    }
    
    return result;
}

bool json_has_key(const nlohmann::json& json, const std::string& key) {
    return json.contains(key);
}

std::optional<std::string> json_get_string(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_string()) {
        return json[key].get<std::string>();
    }
    return std::nullopt;
}

std::optional<int> json_get_int(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_number()) {
        return json[key].get<int>();
    }
    return std::nullopt;
}

std::optional<double> json_get_double(const nlohmann::json& json, const std::string& key) {
    if (json.contains(key) && json[key].is_number()) {
        return json[key].get<double>();
    }
    return std::nullopt;
}

std::string url_encode(const std::string& str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    
    for (char c : str) {
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    
    return escaped.str();
}

std::string url_decode(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 < str.size()) {
                int value = 0;
                std::istringstream iss(str.substr(i + 1, 2));
                if (iss >> std::hex >> value) {
                    result += static_cast<char>(value);
                    i += 2;
                    continue;
                }
            }
        }
        result += str[i];
    }
    
    return result;
}

} // namespace utils
} // namespace job_service
