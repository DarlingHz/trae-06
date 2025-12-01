#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace util {

// 时间相关工具函数
namespace time {
    // 解析ISO 8601格式的时间字符串
    bool parseIsoString(const std::string& iso_str, struct tm& tm_out);
    // 将struct tm转换为ISO 8601格式的时间字符串
    std::string toIsoString(const struct tm& tm);
    // 计算两个时间之间的小时差
    double calculateHoursDiff(const struct tm& start_tm, const struct tm& end_tm);
    // 从ISO时间字符串中提取日期部分（YYYY-MM-DD）
    std::string extractDateFromIsoString(const std::string& iso_str);
}

// JSON相关工具函数
namespace json {
    // 将JSON对象转换为字符串
    std::string toString(const ::json& json_obj);
    // 将字符串转换为JSON对象
    ::json fromString(const std::string& json_str);
}

// 字符串相关工具函数
namespace string {
    // 分割字符串
    std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    // 去除字符串首尾的空白字符
    std::string trim(const std::string& str);
    // 将字符串转换为小写
    std::string toLower(const std::string& str);
    // 将字符串转换为大写
    std::string toUpper(const std::string& str);
}

// 加密相关工具函数
namespace crypto {
    // 计算SHA256哈希
    std::string sha256(const std::string& input);
    // 生成随机字符串
    std::string generateRandomString(int length);
}

} // namespace util

#endif // UTILS_H
