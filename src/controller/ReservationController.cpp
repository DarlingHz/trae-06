#include "ReservationController.h"
#include "../util/Logger.h"
#include "../service/UserService.h"
#include <sstream>
#include <algorithm>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

ReservationController::ReservationController(const string& address) : listener(address) {
    reservation_service = make_shared<ReservationService>();
    
    // 绑定HTTP请求处理函数
    listener.support(methods::POST, bind(&ReservationController::handleReserveBook, this, placeholders::_1));
    listener.support(methods::DEL, bind(&ReservationController::handleCancelReservation, this, placeholders::_1));
    listener.support(methods::GET, bind(&ReservationController::handleGetReservationRecord, this, placeholders::_1));
    listener.support(methods::GET, bind(&ReservationController::handleGetUserReservations, this, placeholders::_1));
    listener.support(methods::GET, bind(&ReservationController::handleGetBookReservations, this, placeholders::_1));
    listener.support(methods::GET, bind(&ReservationController::handleGetBookReservationQueueLength, this, placeholders::_1));
    listener.support(methods::GET, bind(&ReservationController::handleGetUserReservationQueuePosition, this, placeholders::_1));
    listener.support(methods::POST, bind(&ReservationController::handleScanExpiredReservationRecords, this, placeholders::_1));
}

ReservationController::~ReservationController() {
    stop();
}

void ReservationController::start() {
    try {
        listener.open().wait();
        Logger::info("ReservationController HTTP server started at " + listener.uri().to_string());
    } catch (const exception& e) {
        Logger::error("Failed to start ReservationController HTTP server: " + string(e.what()));
        throw;
    }
}

void ReservationController::stop() {
    try {
        listener.close().wait();
        Logger::info("ReservationController HTTP server stopped");
    } catch (const exception& e) {
        Logger::error("Failed to stop ReservationController HTTP server: " + string(e.what()));
        throw;
    }
}

void ReservationController::handleReserveBook(http_request request) {
    Logger::info("Received reserve book request");
    
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
            if (!body.has_field("book_id")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少图书ID参数");
                return;
            }
            
            int book_id = body["book_id"].as_integer();
            
            // 调用预约服务预约图书
            int reservation_id = reservation_service->reserveBook(user_id, book_id);
            if (reservation_id == -1) {
                sendResponse(request, status_codes::BadRequest, 400, "预约图书失败，可能的原因：图书不存在、图书已被预约、用户已达到预约上限、用户有逾期未还的图书");
                return;
            }
            
            // 构造响应数据
            web::json::value data;
            data["reservation_id"] = reservation_id;
            
            sendResponse(request, status_codes::Created, 201, "预约图书成功", data);
            
        } catch (const exception& e) {
            Logger::error("Error handling reserve book request: " + string(e.what()));
            sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
        }
    }).wait();
}

void ReservationController::handleCancelReservation(http_request request) {
    Logger::info("Received cancel reservation request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    // 获取请求参数
    auto query_params = request.relative_uri().query();
    int reservation_id = -1;
    
    // 解析查询参数
    stringstream ss(query_params);
    string param;
    while (getline(ss, param, '&')) {
        size_t pos = param.find('=');
        if (pos != string::npos) {
            string key = param.substr(0, pos);
            string value = param.substr(pos + 1);
            
            if (key == "reservation_id") {
                reservation_id = stoi(value);
            }
        }
    }
    
    // 验证请求参数
    if (reservation_id == -1) {
        sendResponse(request, status_codes::BadRequest, 400, "缺少预约记录ID参数");
        return;
    }
    
    try {
        // 调用预约服务取消预约
        bool success = reservation_service->cancelReservation(reservation_id);
        if (!success) {
            sendResponse(request, status_codes::BadRequest, 400, "取消预约失败，可能的原因：预约记录不存在、预约已被取消");
            return;
        }
        
        sendResponse(request, status_codes::OK, 200, "取消预约成功");
        
    } catch (const exception& e) {
        Logger::error("Error handling cancel reservation request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void ReservationController::handleGetReservationRecord(http_request request) {
    Logger::info("Received get reservation record request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    try {
        // 获取请求参数
        auto query_params = request.relative_uri().query();
        int reservation_id = -1;
        
        // 解析查询参数
        stringstream ss(query_params);
        string param;
        while (getline(ss, param, '&')) {
            size_t pos = param.find('=');
            if (pos != string::npos) {
                string key = param.substr(0, pos);
                string value = param.substr(pos + 1);
                
                if (key == "reservation_id") {
                    reservation_id = stoi(value);
                }
            }
        }
        
        // 验证请求参数
        if (reservation_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少预约记录ID参数");
            return;
        }
        
        // 调用预约服务获取预约记录
        auto reservation_record = reservation_service->getReservationRecordById(reservation_id);
        if (!reservation_record) {
            sendResponse(request, status_codes::NotFound, 404, "预约记录不存在");
            return;
        }
        
        // 检查用户是否有权限查看该预约记录
        if (role != "admin" && reservation_record->getUserId() != user_id) {
            sendResponse(request, status_codes::Forbidden, 403, "禁止访问该预约记录");
            return;
        }
        
        // 构造响应数据
        web::json::value data;
        data["reservation_id"] = reservation_record->getId();
        data["user_id"] = reservation_record->getUserId();
        data["book_id"] = reservation_record->getBookId();
        data["reservation_date"] = web::json::value::string(reservation_record->getReservationDate());
        data["expire_date"] = web::json::value::string(reservation_record->getExpireDate());
        data["status"] = web::json::value::string(reservation_record->getStatus());
        data["queue_position"] = reservation_record->getQueuePosition();
        data["created_at"] = web::json::value::string(reservation_record->getCreatedAt());
        data["updated_at"] = web::json::value::string(reservation_record->getUpdatedAt());
        
        sendResponse(request, status_codes::OK, 200, "获取预约记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get reservation record request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void ReservationController::handleGetUserReservations(http_request request) {
    Logger::info("Received get user reservation records request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    try {
        // 获取请求参数
        auto query_params = request.relative_uri().query();
        int target_user_id = -1;
        string status;
        int page = 1;
        int page_size = 10;
        
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
                } else if (key == "status") {
                    status = value;
                } else if (key == "page") {
                    page = stoi(value);
                } else if (key == "page_size") {
                    page_size = stoi(value);
                }
            }
        }
        
        // 如果是普通用户，只能查看自己的预约记录
        if (role != "admin") {
            if (target_user_id == -1) {
                target_user_id = user_id;
            } else if (target_user_id != user_id) {
                sendResponse(request, status_codes::Forbidden, 403, "禁止访问其他用户的预约记录");
                return;
            }
        }
        
        // 如果是管理员，必须指定用户ID
        if (role == "admin" && target_user_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少用户ID参数");
            return;
        }
        
        // 调用预约服务获取用户预约记录
        auto reservation_records = reservation_service->getUserReservationRecords(target_user_id, status, page, page_size);
        int total = reservation_service->getReservationRecordCount(target_user_id, -1, status);
        
        // 构造响应数据
        web::json::value data;
        web::json::value reservation_record_array = web::json::value::array();
        
        for (const auto& reservation_record : reservation_records) {
            web::json::value reservation_record_obj;
            reservation_record_obj["reservation_id"] = reservation_record->getId();
            reservation_record_obj["user_id"] = reservation_record->getUserId();
            reservation_record_obj["book_id"] = reservation_record->getBookId();
            reservation_record_obj["reservation_date"] = web::json::value::string(reservation_record->getReservationDate());
            reservation_record_obj["expire_date"] = web::json::value::string(reservation_record->getExpireDate());
            reservation_record_obj["status"] = web::json::value::string(reservation_record->getStatus());
            reservation_record_obj["queue_position"] = reservation_record->getQueuePosition();
            reservation_record_obj["created_at"] = web::json::value::string(reservation_record->getCreatedAt());
            reservation_record_obj["updated_at"] = web::json::value::string(reservation_record->getUpdatedAt());
            reservation_record_array[reservation_record_array.size()] = reservation_record_obj;
        }
        
        data["reservation_records"] = reservation_record_array;
        data["total"] = total;
        data["page"] = page;
        data["page_size"] = page_size;
        
        sendResponse(request, status_codes::OK, 200, "获取用户预约记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get user reservation records request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void ReservationController::handleGetBookReservations(http_request request) {
    Logger::info("Received get book reservation records request");
    
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
        int book_id = -1;
        string status;
        int page = 1;
        int page_size = 10;
        
        // 解析查询参数
        stringstream ss(query_params);
        string param;
        while (getline(ss, param, '&')) {
            size_t pos = param.find('=');
            if (pos != string::npos) {
                string key = param.substr(0, pos);
                string value = param.substr(pos + 1);
                
                if (key == "book_id") {
                    book_id = stoi(value);
                } else if (key == "status") {
                    status = value;
                } else if (key == "page") {
                    page = stoi(value);
                } else if (key == "page_size") {
                    page_size = stoi(value);
                }
            }
        }
        
        // 验证请求参数
        if (book_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少图书ID参数");
            return;
        }
        
        // 调用预约服务获取图书预约记录
        auto reservation_records = reservation_service->getBookReservationRecords(book_id, status);
        int total = reservation_service->getReservationRecordCount(-1, book_id, status);
        
        // 构造响应数据
        web::json::value data;
        web::json::value reservation_record_array = web::json::value::array();
        
        for (const auto& reservation_record : reservation_records) {
            web::json::value reservation_record_obj;
            reservation_record_obj["reservation_id"] = reservation_record->getId();
            reservation_record_obj["user_id"] = reservation_record->getUserId();
            reservation_record_obj["book_id"] = reservation_record->getBookId();
            reservation_record_obj["reservation_date"] = web::json::value::string(reservation_record->getReservationDate());
            reservation_record_obj["expire_date"] = web::json::value::string(reservation_record->getExpireDate());
            reservation_record_obj["status"] = web::json::value::string(reservation_record->getStatus());
            reservation_record_obj["queue_position"] = reservation_record->getQueuePosition();
            reservation_record_obj["created_at"] = web::json::value::string(reservation_record->getCreatedAt());
            reservation_record_obj["updated_at"] = web::json::value::string(reservation_record->getUpdatedAt());
            reservation_record_array[reservation_record_array.size()] = reservation_record_obj;
        }
        
        data["reservation_records"] = reservation_record_array;
        data["total"] = total;
        data["page"] = page;
        data["page_size"] = page_size;
        
        sendResponse(request, status_codes::OK, 200, "获取图书预约记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get book reservation records request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void ReservationController::handleGetBookReservationQueueLength(http_request request) {
    Logger::info("Received get book reservation queue length request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    try {
        // 获取请求参数
        auto query_params = request.relative_uri().query();
        int book_id = -1;
        
        // 解析查询参数
        stringstream ss(query_params);
        string param;
        while (getline(ss, param, '&')) {
            size_t pos = param.find('=');
            if (pos != string::npos) {
                string key = param.substr(0, pos);
                string value = param.substr(pos + 1);
                
                if (key == "book_id") {
                    book_id = stoi(value);
                }
            }
        }
        
        // 验证请求参数
        if (book_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少图书ID参数");
            return;
        }
        
        // 调用预约服务获取图书预约队列长度
        int queue_length = reservation_service->getBookReservationQueueLength(book_id, "pending");
        
        // 构造响应数据
        web::json::value data;
        data["queue_length"] = queue_length;
        
        sendResponse(request, status_codes::OK, 200, "获取图书预约队列长度成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get book reservation queue length request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void ReservationController::handleGetUserReservationQueuePosition(http_request request) {
    Logger::info("Received get user reservation queue position request");
    
    // 验证用户身份
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role)) {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问");
        return;
    }
    
    try {
        // 获取请求参数
        auto query_params = request.relative_uri().query();
        int book_id = -1;
        
        // 解析查询参数
        stringstream ss(query_params);
        string param;
        while (getline(ss, param, '&')) {
            size_t pos = param.find('=');
            if (pos != string::npos) {
                string key = param.substr(0, pos);
                string value = param.substr(pos + 1);
                
                if (key == "book_id") {
                    book_id = stoi(value);
                }
            }
        }
        
        // 验证请求参数
        if (book_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少图书ID参数");
            return;
        }
        
        // 调用预约服务获取用户预约队列位置
        int queue_position = reservation_service->getUserReservationQueuePosition(user_id, book_id, "pending");
        if (queue_position == -1) {
            sendResponse(request, status_codes::NotFound, 404, "用户未预约该图书");
            return;
        }
        
        // 构造响应数据
        web::json::value data;
        data["queue_position"] = queue_position;
        
        sendResponse(request, status_codes::OK, 200, "获取用户预约队列位置成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get user reservation queue position request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void ReservationController::handleScanExpiredReservationRecords(http_request request) {
    Logger::info("Received scan expired reservation records request");
    
    // 验证用户身份（需要管理员权限）
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role) || role != "admin") {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问，需要管理员权限");
        return;
    }
    
    try {
        // 调用预约服务扫描过期预约记录
        auto expired_reservation_records = reservation_service->scanExpiredReservationRecords();
        
        // 构造响应数据
        web::json::value data;
        web::json::value expired_reservation_record_array = web::json::value::array();
        
        for (const auto& expired_reservation_record : expired_reservation_records) {
            web::json::value expired_reservation_record_obj;
            expired_reservation_record_obj["reservation_id"] = expired_reservation_record->getId();
            expired_reservation_record_obj["user_id"] = expired_reservation_record->getUserId();
            expired_reservation_record_obj["book_id"] = expired_reservation_record->getBookId();
            expired_reservation_record_obj["reservation_date"] = web::json::value::string(expired_reservation_record->getReservationDate());
            expired_reservation_record_obj["expire_date"] = web::json::value::string(expired_reservation_record->getExpireDate());
            expired_reservation_record_obj["status"] = web::json::value::string(expired_reservation_record->getStatus());
            expired_reservation_record_obj["queue_position"] = expired_reservation_record->getQueuePosition();
            expired_reservation_record_obj["created_at"] = web::json::value::string(expired_reservation_record->getCreatedAt());
            expired_reservation_record_obj["updated_at"] = web::json::value::string(expired_reservation_record->getUpdatedAt());
            expired_reservation_record_array[expired_reservation_record_array.size()] = expired_reservation_record_obj;
        }
        
        data["expired_reservation_records"] = expired_reservation_record_array;
        
        sendResponse(request, status_codes::OK, 200, "扫描过期预约记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling scan expired reservation records request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

bool ReservationController::authenticateUser(http_request request, int& user_id, string& role) {
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
        UserService user_service;
        int verified_user_id = user_service.verifyJWTToken(token);
        if (verified_user_id == -1) {
            Logger::error("Invalid JWT Token");
            return false;
        }
        
        // 获取用户信息
        auto user = user_service.getUserInfo(verified_user_id);
        if (!user) {
            Logger::error("User not found");
            return false;
        }
        
        // 设置用户ID和角色
        user_id = verified_user_id;
        role = user->getRole();
        
        return true;
        
    } catch (const exception& e) {
        Logger::error("Error authenticating user: " + string(e.what()));
        return false;
    }
}

void ReservationController::sendResponse(http_request request, http::status_code status, int code, const string& message, const web::json::value& data) {
    try {
        // 构造响应体
        web::json::value response;
        response["code"] = code;
        response["message"] = web::json::value::string(message);
        response["data"] = data;
        
        // 发送响应
        http_response http_response(static_cast<http::status_code>(status));
        http_response.headers().add("Content-Type", "application/json; charset=utf-8");
        http_response.set_body(response);
        request.reply(http_response).wait();
        
        Logger::info("Sent response to client: code=" + to_string(code) + ", message=" + message);
        
    } catch (const exception& e) {
        Logger::error("Error sending response to client: " + string(e.what()));
        throw;
    }
}