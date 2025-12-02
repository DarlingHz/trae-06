#include "UserController.h"
#include "../util/Logger.h"
#include <sstream>
#include <algorithm>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

UserController::UserController(const string& address) : listener(address) {
    user_service = make_shared<UserService>();
    
    // 绑定HTTP请求处理函数
    listener.support(methods::POST, bind(&UserController::handleRegister, this, placeholders::_1));
    listener.support(methods::POST, bind(&UserController::handleLogin, this, placeholders::_1));
    listener.support(methods::PUT, bind(&UserController::handleUpdateUserInfo, this, placeholders::_1));
    listener.support(methods::PUT, bind(&UserController::handleUpdateUserPassword, this, placeholders::_1));
    listener.support(methods::GET, bind(&UserController::handleGetUserInfo, this, placeholders::_1));
    listener.support(methods::GET, bind(&UserController::handleGetAllUsers, this, placeholders::_1));
    listener.support(methods::PUT, bind(&UserController::handleToggleUserStatus, this, placeholders::_1));
}

UserController::~UserController() {
    stop();
}

void UserController::start() {
    try {
        listener.open().wait();
        Logger::info("UserController HTTP server started at " + listener.uri().to_string());
    } catch (const exception& e) {
        Logger::error("Failed to start UserController HTTP server: " + string(e.what()));
        throw;
    }
}

void UserController::stop() {
    try {
        listener.close().wait();
        Logger::info("UserController HTTP server stopped");
    } catch (const exception& e) {
        Logger::error("Failed to stop UserController HTTP server: " + string(e.what()));
        throw;
    }
}

void UserController::handleRegister(http_request request) {
    Logger::info("Received user registration request");
    
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 验证请求参数
            if (!body.has_field("username") || !body.has_field("password") || !body.has_field("email")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }
            
            string username = body["username"].as_string();
            string password = body["password"].as_string();
            string email = body["email"].as_string();
            string phone = body.has_field("phone") ? body["phone"].as_string() : "";
            string address = body.has_field("address") ? body["address"].as_string() : "";
            
            // 调用用户服务注册用户
            // 创建用户对象并设置属性
            User user;
            user.setUsername(username);
            user.setEmail(email);
            
            // 调用用户服务注册用户
            bool success = user_service->registerUser(user, password);
            if (!success) {
                sendResponse(request, status_codes::Conflict, 409, "用户名或邮箱已存在");
                return;
            }
            
            // 构造响应数据
            web::json::value data;
            data["username"] = web::json::value::string(username);
            data["email"] = web::json::value::string(email);
            // User对象中没有phone和address属性，所以不添加这些字段
            
            sendResponse(request, status_codes::Created, 201, "注册成功", data);
            
        } catch (const exception& e) {
            Logger::error("Error handling user registration request: " + string(e.what()));
            sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
        }
    }).wait();
}

void UserController::handleLogin(http_request request) {
    Logger::info("Received user login request");
    
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 验证请求参数
            if (!body.has_field("username") || !body.has_field("password")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }
            
            string username = body["username"].as_string();
            string password = body["password"].as_string();
            
            // 调用用户服务登录用户
            auto user = user_service->login(username, password);
            if (!user) {
                sendResponse(request, status_codes::Unauthorized, 401, "用户名或密码错误");
                return;
            }
            
            // 生成JWT Token
            string token = user_service->generateJWTToken(*user);
            if (token.empty()) {
                sendResponse(request, status_codes::InternalError, 500, "生成Token失败");
                return;
            }
            
            // 构造响应数据
            web::json::value data;
            data["user_id"] = user->getId();
            data["username"] = web::json::value::string(user->getUsername());
            data["email"] = web::json::value::string(user->getEmail());
            // User对象中没有phone和address属性，所以不添加这些字段
            data["role"] = web::json::value::string(user->getRole());
            data["status"] = web::json::value::string(user->getStatus());
            data["token"] = web::json::value::string(token);
            
            sendResponse(request, status_codes::OK, 200, "登录成功", data);
            
        } catch (const exception& e) {
            Logger::error("Error handling user login request: " + string(e.what()));
            sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
        }
    }).wait();
}

void UserController::handleUpdateUserInfo(http_request request) {
    Logger::info("Received update user info request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 获取请求参数
              string username = body.has_field("username") ? body["username"].as_string() : "";
              string nickname = body.has_field("nickname") ? body["nickname"].as_string() : "";
              string email = body.has_field("email") ? body["email"].as_string() : "";
              
              // 创建用户对象并更新字段
              User user;
              user.setId(user_id);
              user.setUsername(username);
              user.setNickname(nickname);
              user.setEmail(email);
              
              // 调用用户服务更新用户信息
              bool success = user_service->updateUserInfo(user);
            if (!success) {
                sendResponse(request, status_codes::InternalError, 500, "更新用户信息失败");
                return;
            }
            
            sendResponse(request, status_codes::OK, 200, "更新用户信息成功");
            
        } catch (const exception& e) {
            Logger::error("Error handling update user info request: " + string(e.what()));
            sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
        }
    }).wait();
}

void UserController::handleUpdateUserPassword(http_request request) {
    Logger::info("Received update user password request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 验证请求参数
            if (!body.has_field("old_password") || !body.has_field("new_password")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }
            
            string old_password = body["old_password"].as_string();
            string new_password = body["new_password"].as_string();
            
            // 调用用户服务更新用户密码
            bool success = user_service->updateUserPassword(user_id, old_password, new_password);
            if (!success) {
                sendResponse(request, status_codes::BadRequest, 400, "原密码错误");
                return;
            }
            
            sendResponse(request, status_codes::OK, 200, "更新密码成功");
            
        } catch (const exception& e) {
            Logger::error("Error handling update user password request: " + string(e.what()));
            sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
        }
    }).wait();
}

void UserController::handleGetUserInfo(http_request request) {
    Logger::info("Received get user info request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    try {
        // 调用用户服务获取用户信息
        auto user = user_service->getUserInfo(user_id);
        if (!user) {
            sendResponse(request, status_codes::NotFound, 404, "用户不存在");
            return;
        }
        
        // 构造响应数据
        web::json::value data;
        data["user_id"] = user->getId();
        data["username"] = web::json::value::string(user->getUsername());
        data["email"] = web::json::value::string(user->getEmail());
        // User对象中没有phone和address属性，所以不添加这些字段
        data["role"] = web::json::value::string(user->getRole());
        data["status"] = web::json::value::string(user->getStatus());
        data["created_at"] = web::json::value::string(user->getCreatedAt());
        data["updated_at"] = web::json::value::string(user->getUpdatedAt());
        
        sendResponse(request, status_codes::OK, 200, "获取用户信息成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get user info request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void UserController::handleGetAllUsers(http_request request) {
    Logger::info("Received get all users request");
    
    // 验证用户身份（需要管理员权限）
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role) || role != "admin") {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问，需要管理员权限");
        return;
    }
    
    try {
        // 获取请求参数
        auto query_params = request.relative_uri().query();
        int page = 1;
        int page_size = 10;
        string status;
        
        // 解析查询参数
        stringstream ss(query_params);
        string param;
        while (getline(ss, param, '&')) {
            size_t pos = param.find('=');
            if (pos != string::npos) {
                string key = param.substr(0, pos);
                string value = param.substr(pos + 1);
                
                if (key == "page") {
                    page = stoi(value);
                } else if (key == "page_size") {
                    page_size = stoi(value);
                } else if (key == "status") {
                    status = value;
                }
            }
        }
        
        // 调用用户服务获取所有用户
        auto users = user_service->getAllUsers(page, page_size);
        int total = user_service->getUserCount();
        
        // 构造响应数据
        web::json::value data;
        web::json::value user_array = web::json::value::array();
        
        for (const auto& user : users) {
            web::json::value user_obj;
            user_obj["user_id"] = user->getId();
            user_obj["username"] = web::json::value::string(user->getUsername());
            user_obj["email"] = web::json::value::string(user->getEmail());
            // User对象中没有phone和address属性，所以不添加这些字段
            user_obj["role"] = web::json::value::string(user->getRole());
            user_obj["status"] = web::json::value::string(user->getStatus());
            user_obj["created_at"] = web::json::value::string(user->getCreatedAt());
            user_obj["updated_at"] = web::json::value::string(user->getUpdatedAt());
            user_array[user_array.size()] = user_obj;
        }
        
        data["users"] = user_array;
        data["total"] = total;
        data["page"] = page;
        data["page_size"] = page_size;
        
        sendResponse(request, status_codes::OK, 200, "获取所有用户成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get all users request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void UserController::handleToggleUserStatus(http_request request) {
    Logger::info("Received toggle user status request");
    
    // 验证用户身份（需要管理员权限）
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role) || role != "admin") {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问，需要管理员权限");
        return;
    }
    
    try {
        // 获取请求参数
        auto query_params = request.relative_uri().query();
        int target_user_id = -1;
        
        // 解析查询参数
        stringstream ss(query_params);
        string param;
        while (getline(ss, param, '&')) {
            size_t pos = param.find('=');
            if (pos != string::npos) {
                string key = param.substr(0, pos);
                string value = param.substr(pos + 1);
                
                if (key == "user_id") {
                    target_user_id = stoi(value);
                }
            }
        }
        
        // 验证请求参数
        if (target_user_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少用户ID参数");
            return;
        }
        
        // 调用用户服务切换用户状态
        bool success = user_service->toggleUserStatus(target_user_id, "active");
        if (!success) {
            sendResponse(request, status_codes::InternalError, 500, "切换用户状态失败");
            return;
        }
        
        sendResponse(request, status_codes::OK, 200, "切换用户状态成功");
        
    } catch (const exception& e) {
        Logger::error("Error handling toggle user status request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

bool UserController::authenticateUser(http_request request, int& user_id, string& role) {
    try {
        // 获取Authorization头部
        auto auth_header = request.headers().find("Authorization");
        if (auth_header == request.headers().end()) {
            Logger::error("Authorization header not found");
            return false;
        }
        
        // 解析Authorization头部（Bearer Token）
        string auth_value = auth_header->second;
        if (auth_value.substr(0, 7) != "Bearer ") {
            Logger::error("Invalid Authorization header format");
            return false;
        }
        
        string token = auth_value.substr(7);
        
        // 验证JWT Token
        int verified_user_id = user_service->verifyJWTToken(token);
        if (verified_user_id == -1) {
            Logger::error("Invalid JWT Token");
            return false;
        }
        user_id = verified_user_id;
        role = "user"; // 默认角色为user
        
        return true;
        
    } catch (const exception& e) {
        Logger::error("Error authenticating user: " + string(e.what()));
        return false;
    }
}

void UserController::sendResponse(http_request request, http::status_code status, int code, const string& message, const web::json::value& data) {
    try {
        // 构造响应体
        web::json::value response;
        response["code"] = code;
        response["message"] = web::json::value::string(message);
        response["data"] = data;
        
        // 发送响应
        http_response http_response(status);
        http_response.headers().add("Content-Type", "application/json; charset=utf-8");
        http_response.set_body(response);
        request.reply(http_response).wait();
        
        Logger::info("Sent response to client: code=" + to_string(code) + ", message=" + message);
        
    } catch (const exception& e) {
        Logger::error("Error sending response to client: " + string(e.what()));
        throw;
    }
}