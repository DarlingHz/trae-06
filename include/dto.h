#ifndef DTO_H
#define DTO_H

#include <string>
#include <optional>
#include "json_utils.h"

// 用户DTO
struct UserDTO {
    int id = 0;
    std::string name;
    std::string email;
    std::optional<std::string> phone;
    std::string role;
    std::string created_at;
    std::string password_hash;
};

// 注册请求
struct RegisterRequest {
    std::string name;
    std::string email;
    std::string password;
    std::optional<std::string> phone;
};

// 登录请求
struct LoginRequest {
    std::string email;
    std::string password;
};

// 丢失物品DTO
struct LostItemDTO {
    int id = 0;
    int owner_user_id;
    std::string title;
    std::string description;
    std::string category;
    std::string lost_time;
    std::string lost_location;
    std::string status;
    std::string created_at;
    std::string updated_at;
};

// 创建丢失物品请求
struct CreateLostItemRequest {
    std::string title;
    std::string description;
    std::string category;
    std::string lost_time;
    std::string lost_location;
};

// 更新丢失物品请求
struct UpdateLostItemRequest {
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<std::string> category;
    std::optional<std::string> lost_time;
    std::optional<std::string> lost_location;
    std::optional<std::string> status;
};

// 捡到物品DTO
struct FoundItemDTO {
    int id = 0;
    int finder_user_id;
    std::string title;
    std::string description;
    std::string category;
    std::string found_time;
    std::string found_location;
    std::string keep_place;
    std::string status;
    std::string created_at;
    std::string updated_at;
};

// 创建捡到物品请求
struct CreateFoundItemRequest {
    std::string title;
    std::string description;
    std::string category;
    std::string found_time;
    std::string found_location;
    std::string keep_place;
};

// 更新捡到物品请求
struct UpdateFoundItemRequest {
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<std::string> category;
    std::optional<std::string> found_time;
    std::optional<std::string> found_location;
    std::optional<std::string> keep_place;
    std::optional<std::string> status;
};

// 认领DTO
struct ClaimDTO {
    int id = 0;
    int lost_item_id;
    int found_item_id;
    int claimant_user_id;
    std::string status;
    std::string evidence_text;
    std::string created_at;
    std::string updated_at;
};

// 创建认领请求
struct CreateClaimRequest {
    int lost_item_id;
    int found_item_id;
    std::string evidence_text;
};

// 通知DTO
struct NotificationDTO {
    int id = 0;
    int user_id;
    std::string message;
    std::string type;
    bool is_read;
    std::string created_at;
};

// 统计数据
struct StatData {
    int open_lost_items = 0;
    int open_found_items = 0;
    int lost_items_7d = 0;
    int found_items_7d = 0;
    int claims_7d = 0;
};

// JSON序列化/反序列化模板
template <typename T>
json::JsonValue to_json(const T& obj) {
    json::JsonValue j;
    // 由具体类型实现
    return j;
}

template <typename T>
T from_json(const json::JsonValue& j) {
    T obj;
    // 由具体类型实现
    return obj;
}

#endif // DTO_H