#pragma once

#include <string>
#include <vector>
#include <ctime>

namespace utils {

// URL parsing utilities
std::string extract_domain(const std::string& url);
std::string normalize_url(const std::string& url);

// String utilities
bool starts_with(const std::string& str, const std::string& prefix);
bool ends_with(const std::string& str, const std::string& suffix);
std::vector<std::string> split(const std::string& str, char delimiter);
std::vector<std::string> split(const std::string& str, const std::string& delimiter);
std::string trim(const std::string& str);
std::string to_lower(const std::string& str);
std::string to_upper(const std::string& str);

// Time utilities
std::string format_time(time_t timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S");
std::string current_time_str();
std::time_t parse_time(const std::string& str, const std::string& format = "%Y-%m-%d %H:%M:%S");

// Random utilities
std::string generate_random_string(size_t length);

// File utilities
bool file_exists(const std::string& path);
std::string read_file(const std::string& path);
bool write_file(const std::string& path, const std::string& content);

// JSON utilities
std::string escape_json(const std::string& str);

} // namespace utils
