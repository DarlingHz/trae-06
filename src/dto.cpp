#include "dto.h"
#include "json_utils.h"

// UserDTO的JSON序列化
template <>
json::JsonValue to_json<UserDTO>(const UserDTO& obj) {
    json::JsonValue j;
    j["id"] = obj.id;
    j["name"] = obj.name;
    j["email"] = obj.email;
    if (obj.phone) {
        j["phone"] = *obj.phone;
    }
    j["role"] = obj.role;
    j["created_at"] = obj.created_at;
    return j;
}

// UserDTO的JSON反序列化
template <>
UserDTO from_json<UserDTO>(const json::JsonValue& j) {
    UserDTO obj;
    obj.id = j["id"].as_int();
    obj.name = j["name"].as_string();
    obj.email = j["email"].as_string();
    if (j.has("phone")) {
        obj.phone = j["phone"].as_string();
    }
    obj.role = j["role"].as_string();
    obj.created_at = j["created_at"].as_string();
    return obj;
}

// RegisterRequest的JSON序列化
template <>
json::JsonValue to_json<RegisterRequest>(const RegisterRequest& obj) {
    json::JsonValue j;
    j["name"] = obj.name;
    j["email"] = obj.email;
    j["password"] = obj.password;
    if (obj.phone) {
        j["phone"] = *obj.phone;
    }
    return j;
}

// RegisterRequest的JSON反序列化
template <>
RegisterRequest from_json<RegisterRequest>(const json::JsonValue& j) {
    RegisterRequest obj;
    obj.name = j["name"].as_string();
    obj.email = j["email"].as_string();
    obj.password = j["password"].as_string();
    if (j.has("phone")) {
        obj.phone = j["phone"].as_string();
    }
    return obj;
}

// LoginRequest的JSON序列化
template <>
json::JsonValue to_json<LoginRequest>(const LoginRequest& obj) {
    json::JsonValue j;
    j["email"] = obj.email;
    j["password"] = obj.password;
    return j;
}

// LoginRequest的JSON反序列化
template <>
LoginRequest from_json<LoginRequest>(const json::JsonValue& j) {
    LoginRequest obj;
    obj.email = j["email"].as_string();
    obj.password = j["password"].as_string();
    return obj;
}

// LostItem的JSON序列化
template <>
json::JsonValue to_json<LostItemDTO>(const LostItemDTO& obj) {
    json::JsonValue j;
    j["id"] = obj.id;
    j["owner_user_id"] = obj.owner_user_id;
    j["title"] = obj.title;
    j["description"] = obj.description;
    j["category"] = obj.category;
    j["lost_time"] = obj.lost_time;
    j["lost_location"] = obj.lost_location;
    j["created_at"] = obj.created_at;
    j["status"] = obj.status;
    return j;
}

// LostItem的JSON反序列化
template <>
LostItemDTO from_json<LostItemDTO>(const json::JsonValue& j) {
    LostItemDTO obj;
    obj.id = j["id"].as_int();
    obj.owner_user_id = j["owner_user_id"].as_int();
    obj.title = j["title"].as_string();
    obj.description = j["description"].as_string();
    obj.category = j["category"].as_string();
    obj.lost_location = j["lost_location"].as_string();
    obj.status = j["status"].as_string();
    obj.created_at = j["created_at"].as_string();
    obj.status = j["status"].as_string();
    return obj;
}

// FoundItem的JSON序列化
template <>
json::JsonValue to_json<FoundItemDTO>(const FoundItemDTO& obj) {
    json::JsonValue j;
    j["id"] = obj.id;
    j["finder_user_id"] = obj.finder_user_id;
    j["title"] = obj.title;
    j["description"] = obj.description;
    j["category"] = obj.category;
    j["found_time"] = obj.found_time;
    j["found_location"] = obj.found_location;
    j["keep_place"] = obj.keep_place;
    j["created_at"] = obj.created_at;
    j["status"] = obj.status;
    return j;
}

// FoundItem的JSON反序列化
template <>
FoundItemDTO from_json<FoundItemDTO>(const json::JsonValue& j) {
    FoundItemDTO obj;
    obj.id = j["id"].as_int();
    obj.finder_user_id = j["finder_user_id"].as_int();
    obj.title = j["title"].as_string();
    obj.description = j["description"].as_string();
    obj.category = j["category"].as_string();
    obj.found_time = j["found_time"].as_string();
    obj.found_location = j["found_location"].as_string();
    obj.keep_place = j["keep_place"].as_string();
    obj.created_at = j["created_at"].as_string();
    obj.status = j["status"].as_string();
    return obj;
}

// Claim的JSON序列化
template <>
json::JsonValue to_json<ClaimDTO>(const ClaimDTO& obj) {
    json::JsonValue j;
    j["id"] = obj.id;
    j["lost_item_id"] = obj.lost_item_id;
    j["found_item_id"] = obj.found_item_id;
    j["evidence_text"] = obj.evidence_text;
    j["status"] = obj.status;
    j["created_at"] = obj.created_at;
    return j;
}

// Claim的JSON反序列化
template <>
ClaimDTO from_json<ClaimDTO>(const json::JsonValue& j) {
    ClaimDTO obj;
    obj.id = j["id"].as_int();
    obj.claimant_user_id = j["claimant_user_id"].as_int();
    obj.found_item_id = j["found_item_id"].as_int();
    obj.evidence_text = j["evidence_text"].as_string();
    obj.status = j["status"].as_string();
    obj.created_at = j["created_at"].as_string();
    return obj;
}

// StatData的JSON序列化
template <>
json::JsonValue to_json<StatData>(const StatData& obj) {
    json::JsonValue j;
    j["open_lost_items"] = obj.open_lost_items;
    j["open_found_items"] = obj.open_found_items;
    j["lost_items_7d"] = obj.lost_items_7d;
    j["found_items_7d"] = obj.found_items_7d;
    j["claims_7d"] = obj.claims_7d;
    return j;
}

// StatData的JSON反序列化
template <>
StatData from_json<StatData>(const json::JsonValue& j) {
    StatData obj;
    obj.open_lost_items = j["open_lost_items"].as_int();
    obj.open_found_items = j["open_found_items"].as_int();
    obj.lost_items_7d = j["lost_items_7d"].as_int();
    obj.found_items_7d = j["found_items_7d"].as_int();
    obj.claims_7d = j["claims_7d"].as_int();
    return obj;
}