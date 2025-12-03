#include <httplib.h>
#include "config.h"
#include "database.h"
#include "logger.h"
#include "json_utils.h"
#include "auth.h"
#include "user_service.h"
#include "lost_item_service.h"
#include "found_item_service.h"
#include "claim_service.h"
#include "notification_service.h"
#include "dashboard_service.h"
#include "dto.h"

using namespace httplib;

#ifndef SETUP_ROUTES_DEFINED
#define SETUP_ROUTES_DEFINED
void setup_routes(Server& svr) {
    // 健康检查
    svr.Get("/api/health", [](const Request& req, Response& res) {
        json::JsonValue data;
        data["status"] = "ok";
        data["database"] = Database::instance().get_db() != nullptr ? "connected" : "disconnected";
        
        json::JsonValue json_response;
        json_response["code"] = 0;
        json_response["message"] = "ok";
        json_response["data"] = data;
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
    
    // 用户注册
    svr.Post("/api/users/register", [](const Request& req, Response& res) {
        try {
            json::JsonValue j = json::Parser::parse(req.body);
            RegisterRequest request;
            request.name = j["name"].as_string();
            request.email = j["email"].as_string();
            request.password = j["password"].as_string();
            if (j.has("phone")) {
                request.phone = j["phone"].as_string();
            }
            
            auto user = UserService::instance().register_user(request);
            if (user.has_value()) {
                json::JsonValue data;
                data["id"] = user->id;
                data["name"] = user->name;
                data["email"] = user->email;
                data["role"] = user->role;
                
                json::JsonValue json_response;
                json_response["code"] = 0;
                json_response["message"] = "ok";
                json_response["data"] = data;
                
                res.set_content(json::Serializer::serialize(json_response), "application/json");
                res.status = 201;
            } else {
                res.status = 400;
                json::JsonValue json_response;
                json_response["code"] = 1;
                json_response["message"] = "注册失败";
                res.set_content(json::Serializer::serialize(json_response), "application/json");
            }
        } catch (...) {
            res.status = 400;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "请求参数错误";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
        }
    });
    
    // 用户登录
    svr.Post("/api/users/login", [](const Request& req, Response& res) {
        try {
            json::JsonValue j = json::Parser::parse(req.body);
            LoginRequest request;
            request.email = j["email"].as_string();
            request.password = j["password"].as_string();
            
            auto user = UserService::instance().login(request);
            if (user.has_value()) {
                std::string token = AuthManager::instance().generate_token(*user);
                
                json::JsonValue data;
                data["token"] = token;
                std::map<std::string, json::JsonValue> user_map;
                user_map["id"] = user->id;
                user_map["name"] = user->name;
                user_map["email"] = user->email;
                user_map["role"] = user->role;
                data["user"] = json::JsonValue(user_map);
                
                json::JsonValue json_response;
                json_response["code"] = 0;
                json_response["message"] = "ok";
                json_response["data"] = data;
                
                res.set_content(json::Serializer::serialize(json_response), "application/json");
            } else {
                res.status = 401;
                json::JsonValue json_response;
                json_response["code"] = 1;
                json_response["message"] = "邮箱或密码错误";
                res.set_content(json::Serializer::serialize(json_response), "application/json");
            }
        } catch (...) {
            res.status = 400;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "请求参数错误";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
        }
    });
    
    // 获取当前用户信息
    svr.Get("/api/users/me", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto user = UserService::instance().get_user_by_id(user_opt->id);
        if (user.has_value()) {
            json::JsonValue data;
            data["id"] = user->id;
            data["name"] = user->name;
            data["email"] = user->email;
            data["role"] = user->role;
            data["created_at"] = user->created_at;
            
            json::JsonValue json_response;
            json_response["code"] = 0;
            json_response["message"] = "ok";
            json_response["data"] = data;
            
            res.set_content(json::Serializer::serialize(json_response), "application/json");
        } else {
            res.status = 404;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "用户不存在";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
        }
    });
    
    // 创建丢失物品
    svr.Post("/api/lost-items", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        try {
            json::JsonValue j = json::Parser::parse(req.body);
            CreateLostItemRequest request;
            request.title = j["title"].as_string();
            request.description = j["description"].as_string();
            request.category = j["category"].as_string();
            request.lost_time = j["lost_time"].as_string();
            request.lost_location = j["lost_location"].as_string();
            
            auto item = LostItemService::instance().create_lost_item(request, user_opt->id);
            if (item.has_value()) {
                json::JsonValue data;
                data["id"] = item->id;
                data["title"] = item->title;
                data["description"] = item->description;
                data["category"] = item->category;
                data["lost_time"] = item->lost_time;
                data["lost_location"] = item->lost_location;
                data["status"] = item->status;
                data["owner_user_id"] = item->owner_user_id;
                
                json::JsonValue json_response;
                json_response["code"] = 0;
                json_response["message"] = "ok";
                json_response["data"] = data;
                
                res.set_content(json::Serializer::serialize(json_response), "application/json");
                res.status = 201;
            } else {
                res.status = 500;
                json::JsonValue json_response;
                json_response["code"] = 1;
                json_response["message"] = "创建失败";
                res.set_content(json::Serializer::serialize(json_response), "application/json");
            }
        } catch (...) {
            res.status = 400;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "请求参数错误";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
        }
    });
    
    // 获取丢失物品列表
    svr.Get("/api/lost-items", [](const Request& req, Response& res) {
        int page = 1;
        int limit = 10;
        
        auto page_opt = req.get_param_value("page");
        if (!page_opt.empty()) page = std::stoi(page_opt);
        
        auto limit_opt = req.get_param_value("limit");
        if (!limit_opt.empty()) limit = std::stoi(limit_opt);
        
        auto category_opt = req.get_param_value("category");
        auto keyword_opt = req.get_param_value("keyword");
        auto status_opt = req.get_param_value("status");
        
        auto items = LostItemService::instance().get_lost_items(page, limit, category_opt, keyword_opt, status_opt);
        
        json::JsonValue data;
        std::vector<json::JsonValue> items_array;
        for (const auto& item : items) {
            json::JsonValue item_j;
            item_j["id"] = item.id;
            item_j["title"] = item.title;
            item_j["description"] = item.description;
            item_j["category"] = item.category;
            item_j["lost_time"] = item.lost_time;
            item_j["lost_location"] = item.lost_location;
            item_j["status"] = item.status;
            item_j["created_at"] = item.created_at;
            items_array.push_back(item_j);
        }
        data["items"] = json::JsonValue(items_array);
        
        json::JsonValue json_response;
        json_response["code"] = 0;
        json_response["message"] = "ok";
        json_response["data"] = data;
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
    
    // 创建捡到物品
    svr.Post("/api/found-items", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        try {
            json::JsonValue j = json::Parser::parse(req.body);
            CreateFoundItemRequest request;
            request.title = j["title"].as_string();
            request.description = j["description"].as_string();
            request.category = j["category"].as_string();
            request.found_time = j["found_time"].as_string();
            request.found_location = j["found_location"].as_string();
            request.keep_place = j["keep_place"].as_string();
            
            auto item = FoundItemService::instance().create_found_item(request, user_opt->id);
            if (item.has_value()) {
                json::JsonValue data;
                data["id"] = item->id;
                data["title"] = item->title;
                data["description"] = item->description;
                data["category"] = item->category;
                data["found_time"] = item->found_time;
                data["found_location"] = item->found_location;
                data["keep_place"] = item->keep_place;
                data["status"] = item->status;
                
                json::JsonValue json_response;
                json_response["code"] = 0;
                json_response["message"] = "ok";
                json_response["data"] = data;
                
                res.set_content(json::Serializer::serialize(json_response), "application/json");
                res.status = 201;
            } else {
                res.status = 500;
                json::JsonValue json_response;
                json_response["code"] = 1;
                json_response["message"] = "创建失败";
                res.set_content(json::Serializer::serialize(json_response), "application/json");
            }
        } catch (...) {
            res.status = 400;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "请求参数错误";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
        }
    });
    
    // 获取捡到物品列表
    svr.Get("/api/found-items", [](const Request& req, Response& res) {
        int page = 1;
        int limit = 10;
        
        auto page_opt = req.get_param_value("page");
        if (!page_opt.empty()) page = std::stoi(page_opt);
        
        auto limit_opt = req.get_param_value("limit");
        if (!limit_opt.empty()) limit = std::stoi(limit_opt);
        
        auto category_opt = req.get_param_value("category");
        auto keyword_opt = req.get_param_value("keyword");
        auto status_opt = req.get_param_value("status");
        
        auto items = FoundItemService::instance().get_found_items(page, limit, category_opt, keyword_opt, status_opt);
        
        json::JsonValue data;
        std::vector<json::JsonValue> items_array;
        for (const auto& item : items) {
            json::JsonValue item_j;
            item_j["id"] = item.id;
            item_j["title"] = item.title;
            item_j["description"] = item.description;
            item_j["category"] = item.category;
            item_j["found_time"] = item.found_time;
            item_j["found_location"] = item.found_location;
            item_j["keep_place"] = item.keep_place;
            item_j["status"] = item.status;
            item_j["created_at"] = item.created_at;
            items_array.push_back(item_j);
        }
        data["items"] = json::JsonValue(items_array);
        
        json::JsonValue json_response;
        json_response["code"] = 0;
        json_response["message"] = "ok";
        json_response["data"] = data;
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
    
    // 创建认领
    svr.Post("/api/claims", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        try {
            json::JsonValue j = json::Parser::parse(req.body);
            CreateClaimRequest request;
            request.lost_item_id = j["lost_item_id"].as_int();
            request.found_item_id = j["found_item_id"].as_int();
            request.evidence_text = j["evidence_text"].as_string();
            
            auto claim = ClaimService::instance().create_claim(request, user_opt->id);
            if (claim.has_value()) {
                json::JsonValue data;
                data["id"] = claim->id;
                data["lost_item_id"] = claim->lost_item_id;
                data["found_item_id"] = claim->found_item_id;
                data["status"] = claim->status;
                data["evidence_text"] = claim->evidence_text;
                
                json::JsonValue json_response;
                json_response["code"] = 0;
                json_response["message"] = "ok";
                json_response["data"] = data;
                
                res.set_content(json::Serializer::serialize(json_response), "application/json");
                res.status = 201;
            } else {
                res.status = 400;
                json::JsonValue json_response;
                json_response["code"] = 1;
                json_response["message"] = "创建认领失败";
                res.set_content(json::Serializer::serialize(json_response), "application/json");
            }
        } catch (...) {
            res.status = 400;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "请求参数错误";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
        }
    });
    
    // 获取认领列表
    svr.Get("/api/claims", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto status_opt = req.get_param_value("status");
        auto claims = ClaimService::instance().get_claims(user_opt->id, status_opt);
        
        json::JsonValue data;
        std::vector<json::JsonValue> claims_array;
        for (const auto& claim : claims) {
            json::JsonValue claim_j;
            claim_j["id"] = claim.id;
            claim_j["lost_item_id"] = claim.lost_item_id;
            claim_j["found_item_id"] = claim.found_item_id;
            claim_j["status"] = claim.status;
            claim_j["evidence_text"] = claim.evidence_text;
            claim_j["created_at"] = claim.created_at;
            claims_array.push_back(claim_j);
        }
        data["claims"] = json::JsonValue(claims_array);
        
        json::JsonValue json_response;
        json_response["code"] = 0;
        json_response["message"] = "ok";
        json_response["data"] = data;
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
    
    // 审批认领
    svr.Post("/api/claims/(\\d+)/approve", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        if (user_opt->role != "admin" && user_opt->role != "staff") {
            res.status = 403;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "权限不足";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        int claim_id = std::stoi(req.matches[1]);
        bool success = ClaimService::instance().approve_claim(claim_id, *user_opt);
        
        json::JsonValue json_response;
        if (success) {
            json_response["code"] = 0;
            json_response["message"] = "ok";
        } else {
            res.status = 400;
            json_response["code"] = 1;
            json_response["message"] = "审批失败";
        }
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
    
    // 拒绝认领
    svr.Post("/api/claims/(\\d+)/reject", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        if (user_opt->role != "admin" && user_opt->role != "staff") {
            res.status = 403;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "权限不足";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        int claim_id = std::stoi(req.matches[1]);
        bool success = ClaimService::instance().reject_claim(claim_id, *user_opt);
        
        json::JsonValue json_response;
        if (success) {
            json_response["code"] = 0;
            json_response["message"] = "ok";
        } else {
            res.status = 400;
            json_response["code"] = 1;
            json_response["message"] = "拒绝失败";
        }
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
    
    // 获取通知列表
    svr.Get("/api/notifications", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto notifications = NotificationService::instance().get_notifications(user_opt->id);
        
        json::JsonValue data;
        std::vector<json::JsonValue> notifications_array;
        for (const auto& notification : notifications) {
            json::JsonValue notification_j;
            notification_j["id"] = notification.id;
            notification_j["message"] = notification.message;
            notification_j["type"] = notification.type;
            notification_j["is_read"] = notification.is_read;
            notification_j["created_at"] = notification.created_at;
            notifications_array.push_back(notification_j);
        }
        data["notifications"] = json::JsonValue(notifications_array);
        
        json::JsonValue json_response;
        json_response["code"] = 0;
        json_response["message"] = "ok";
        json_response["data"] = data;
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
    
    // 管理员仪表盘
    svr.Get("/api/admin/dashboard", [](const Request& req, Response& res) {
        auto auth_header = req.get_header_value("Authorization");
        if (auth_header.empty()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "未授权";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        auto token_opt = AuthManager::instance().extract_token_from_header(auth_header);
        if (!token_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        auto user_opt = AuthManager::instance().verify_token(*token_opt);
        if (!user_opt.has_value()) {
            res.status = 401;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "无效token";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        if (user_opt->role != "admin") {
            res.status = 403;
            json::JsonValue json_response;
            json_response["code"] = 1;
            json_response["message"] = "权限不足";
            res.set_content(json::Serializer::serialize(json_response), "application/json");
            return;
        }
        
        StatData stats = DashboardService::instance().get_stat_data();
        auto top_categories = DashboardService::instance().get_top_categories(5);
        
        json::JsonValue data;
        data["open_lost_items"] = stats.open_lost_items;
        data["open_found_items"] = stats.open_found_items;
        data["lost_items_7d"] = stats.lost_items_7d;
        data["found_items_7d"] = stats.found_items_7d;
        data["claims_7d"] = stats.claims_7d;
        
        std::vector<json::JsonValue> categories_vector;
        for (const auto& pair : top_categories) {
            json::JsonValue cat;
            cat["category"] = pair.first;
            cat["count"] = pair.second;
            categories_vector.push_back(cat);
        }
        json::JsonValue categories_json = json::JsonValue(categories_vector);
        data["top_categories"] = categories_json;
        
        json::JsonValue json_response;
        json_response["code"] = 0;
        json_response["message"] = "ok";
        json_response["data"] = data;
        
        res.set_content(json::Serializer::serialize(json_response), "application/json");
    });
}
#endif