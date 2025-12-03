#include "utils/Utils.h"
#include <regex>
#include <stdexcept>
#include <fstream>
#include <random>
#include <sstream>
#include <iomanip>

namespace utils {

std::string extract_domain(const std::string& url) {
    static const std::regex url_regex(R"(^(http|https)://([^/]+))");
    std::smatch match;
    
    if(std::regex_search(url, match, url_regex)) {
        return match[2];
    }
    
    return "";
}

std::string normalize_url(const std::string& url) {
    static const std::regex www_regex(R"(^www\.)");
    
    std::string result = url;
    
    // Remove trailing slash
    if(!result.empty() && result.back() == '/') {
        result.pop_back();
    }
    
    // Remove www prefix
    result = std::regex_replace(result, www_regex, "");
    
    return result;
}

bool starts_with(const std::string& str, const std::string& prefix) {
    if(prefix.length() > str.length()) {
        return false;
    }
    
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if(suffix.length() > str.length()) {
        return false;
    }
    
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while(std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while(end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    tokens.push_back(str.substr(start));
    return tokens;
}

std::string trim(const std::string& str) {
    static const std::regex whitespace_regex(R"(^\s+|\s+$)");
    return std::regex_replace(str, whitespace_regex, "");
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    for(auto& c : result) {
        c = std::tolower(c);
    }
    return result;
}

std::string to_upper(const std::string& str) {
    std::string result = str;
    for(auto& c : result) {
        c = std::toupper(c);
    }
    return result;
}

std::string format_time(time_t timestamp, const std::string& format) {
    std::tm tm = *std::localtime(&timestamp);
    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
}

std::string current_time_str() {
    auto now = std::time(nullptr);
    return format_time(now);
}

std::time_t parse_time(const std::string& str, const std::string& format) {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, format.c_str());
    
    if(ss.fail()) {
        throw std::invalid_argument("Failed to parse time");
    }
    
    return std::mktime(&tm);
}

std::string generate_random_string(size_t length) {
    static const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    static std::random_device rd;
    static std::mt19937 generator(rd());
    static std::uniform_int_distribution<int> distribution(0, chars.size() - 1);
    
    std::string result;
    result.reserve(length);
    
    for(size_t i = 0; i < length; ++i) {
        result += chars[distribution(generator)];
    }
    
    return result;
}

bool file_exists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if(!file.is_open()) {
        return false;
    }
    
    file << content;
    return file.good();
}

std::string escape_json(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for(char c : str) {
        switch(c) {
            case '"': result += '\\'; result += '"'; break;
            case '\\': result += '\\'; result += '\\'; break;
            case '\b': result += '\\'; result += 'b'; break;
            case '\f': result += '\\'; result += 'f'; break;
            case '\n': result += '\\'; result += 'n'; break;
            case '\r': result += '\\'; result += 'r'; break;
            case '\t': result += '\\'; result += 't'; break;
            default: result += c; break;
        }
    }
    
    return result;
}

} // namespace utils
