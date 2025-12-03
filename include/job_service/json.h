#ifndef JOB_SERVICE_JSON_H
#define JOB_SERVICE_JSON_H

#include <nlohmann/json.hpp>

// 使用nlohmann的json库
namespace nlohmann {
    // 扩展JSON以支持C++17的optional类型
    template<typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt == std::nullopt) {
                j = nullptr;
            } else {
                j = *opt;
            }
        }
        
        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };
}

#endif // JOB_SERVICE_JSON_H
