#ifndef JOB_SERVICE_UTILS_H
#define JOB_SERVICE_UTILS_H

#include <string>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

namespace job_service {
namespace utils {

// 生成唯一Job ID
std::string generate_job_id();

// 转换时间点到ISO格式字符串
std::string time_to_iso_string(const std::chrono::system_clock::time_point& time);

// 从ISO格式字符串转换到时间点
std::optional<std::chrono::system_clock::time_point> iso_string_to_time(const std::string& str);

// 字符串分割函数
std::vector<std::string> split(const std::string& str, char delimiter);

// 字符串修剪函数
std::string trim(const std::string& str);

// 转换字符串到小写
std::string to_lowercase(const std::string& str);

// 移除字符串中的标点符号
std::string remove_punctuation(const std::string& str);

// 生成随机字符串
std::string generate_random_string(size_t length);

// 检查JSON对象是否包含指定键
bool json_has_key(const nlohmann::json& json, const std::string& key);

// 获取JSON对象中的字符串值
std::optional<std::string> json_get_string(const nlohmann::json& json, const std::string& key);

// 获取JSON对象中的整数值
std::optional<int> json_get_int(const nlohmann::json& json, const std::string& key);

// 获取JSON对象中的浮点数值
std::optional<double> json_get_double(const nlohmann::json& json, const std::string& key);

// URL编码
std::string url_encode(const std::string& str);

// URL解码
std::string url_decode(const std::string& str);

} // namespace utils
} // namespace job_service

#endif // JOB_SERVICE_UTILS_H
