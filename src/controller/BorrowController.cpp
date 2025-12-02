#include "BorrowController.h"
#include "../util/Logger.h"
#include "../service/UserService.h"
#include <sstream>
#include <algorithm>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

BorrowController::BorrowController(const string& address) : listener(address) {
    borrow_service = make_shared<BorrowService>();
    
    // 绑定HTTP请求处理函数
    listener.support(methods::POST, bind(&BorrowController::handleBorrowBook, this, placeholders::_1));
    listener.support(methods::PUT, bind(&BorrowController::handleReturnBook, this, placeholders::_1));
    listener.support(methods::GET, bind(&BorrowController::handleGetBorrowRecord, this, placeholders::_1));
    listener.support(methods::GET, bind(&BorrowController::handleGetUserBorrowRecords, this, placeholders::_1));
    listener.support(methods::GET, bind(&BorrowController::handleGetBookBorrowRecords, this, placeholders::_1));
    listener.support(methods::GET, bind(&BorrowController::handleGetOverdueBorrowRecords, this, placeholders::_1));
    listener.support(methods::POST, bind(&BorrowController::handleScanOverdueBorrowRecords, this, placeholders::_1));
}

BorrowController::~BorrowController() {
    stop();
}

void BorrowController::start() {
    try {
        listener.open().wait();
        Logger::info("BorrowController HTTP server started at " + listener.uri().to_string());
    } catch (const exception& e) {
        Logger::error("Failed to start BorrowController HTTP server: " + string(e.what()));
        throw;
    }
}

void BorrowController::stop() {
    try {
        listener.close().wait();
        Logger::info("BorrowController HTTP server stopped");
    } catch (const exception& e) {
        Logger::error("Failed to stop BorrowController HTTP server: " + string(e.what()));
        throw;
    }
}

void BorrowController::handleBorrowBook(http_request request) {
    Logger::info("Received borrow book request");
    
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
            
            // 调用借阅服务借阅图书
            int borrow_id = borrow_service->borrowBook(user_id, book_id);
            if (borrow_id == -1) {
                sendResponse(request, status_codes::BadRequest, 400, "借阅图书失败，可能的原因：图书不存在、图书已被借出、用户已达到借阅上限、用户有逾期未还的图书");
                return;
            }
            
            // 构造响应数据
            web::json::value data;
            data["borrow_id"] = borrow_id;
            
            sendResponse(request, status_codes::Created, 201, "借阅图书成功", data);
            
        } catch (const exception& e) {
            Logger::error("Error handling borrow book request: " + string(e.what()));
            sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
        }
    }).wait();
}

void BorrowController::handleReturnBook(http_request request) {
    Logger::info("Received return book request");
    
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
            if (!body.has_field("borrow_id")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少借阅记录ID参数");
                return;
            }
            
            int borrow_id = body["borrow_id"].as_integer();
            
            // 调用借阅服务归还图书
            bool success = borrow_service->returnBook(borrow_id);
            if (!success) {
                sendResponse(request, status_codes::BadRequest, 400, "归还图书失败，可能的原因：借阅记录不存在、图书已被归还");
                return;
            }
            
            sendResponse(request, status_codes::OK, 200, "归还图书成功");
            
        } catch (const exception& e) {
            Logger::error("Error handling return book request: " + string(e.what()));
            sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
        }
    }).wait();
}

void BorrowController::handleGetBorrowRecord(http_request request) {
    Logger::info("Received get borrow record request");
    
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
        int borrow_id = -1;
        
        // 解析查询参数
        stringstream ss(query_params);
        string param;
        while (getline(ss, param, '&')) {
            size_t pos = param.find('=');
            if (pos != string::npos) {
                string key = param.substr(0, pos);
                string value = param.substr(pos + 1);
                
                if (key == "borrow_id") {
                    borrow_id = stoi(value);
                }
            }
        }
        
        // 验证请求参数
        if (borrow_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少借阅记录ID参数");
            return;
        }
        
        // 调用借阅服务获取借阅记录
        auto borrow_record = borrow_service->getBorrowRecordById(borrow_id);
        if (!borrow_record) {
            sendResponse(request, status_codes::NotFound, 404, "借阅记录不存在");
            return;
        }
        
        // 检查用户是否有权限查看该借阅记录
        if (role != "admin" && borrow_record->getUserId() != user_id) {
            sendResponse(request, status_codes::Forbidden, 403, "禁止访问该借阅记录");
            return;
        }
        
        // 构造响应数据
        web::json::value data;
        data["borrow_id"] = borrow_record->getId();
        data["user_id"] = borrow_record->getUserId();
        data["book_id"] = borrow_record->getBookId();
        data["borrow_time"] = web::json::value::string(borrow_record->getBorrowDate());
        data["due_time"] = web::json::value::string(borrow_record->getDueDate());
        data["return_time"] = borrow_record->getReturnDate().empty() ? web::json::value::null() : web::json::value::string(borrow_record->getReturnDate());
        data["status"] = web::json::value::string(borrow_record->getStatus());
        data["created_at"] = web::json::value::string(borrow_record->getCreatedAt());
        data["updated_at"] = web::json::value::string(borrow_record->getUpdatedAt());
        
        sendResponse(request, status_codes::OK, 200, "获取借阅记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get borrow record request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void BorrowController::handleGetUserBorrowRecords(http_request request) {
    Logger::info("Received get user borrow records request");
    
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
        
        // 如果是普通用户，只能查看自己的借阅记录
        if (role != "admin") {
            if (target_user_id == -1) {
                target_user_id = user_id;
            } else if (target_user_id != user_id) {
                sendResponse(request, status_codes::Forbidden, 403, "禁止访问其他用户的借阅记录");
                return;
            }
        }
        
        // 如果是管理员，必须指定用户ID
        if (role == "admin" && target_user_id == -1) {
            sendResponse(request, status_codes::BadRequest, 400, "缺少用户ID参数");
            return;
        }
        
        // 调用借阅服务获取用户借阅记录
        auto borrow_records = borrow_service->getUserBorrowRecords(target_user_id, status, page, page_size);
        int total = borrow_service->getBorrowRecordCount(target_user_id, -1, status);
        
        // 构造响应数据
        web::json::value data;
        web::json::value borrow_record_array = web::json::value::array();
        
        for (const auto& borrow_record : borrow_records) {
            web::json::value borrow_record_obj;
            borrow_record_obj["borrow_id"] = borrow_record->getId();
            borrow_record_obj["user_id"] = borrow_record->getUserId();
            borrow_record_obj["book_id"] = borrow_record->getBookId();
            borrow_record_obj["borrow_time"] = web::json::value::string(borrow_record->getBorrowDate());
            borrow_record_obj["due_time"] = web::json::value::string(borrow_record->getDueDate());
            borrow_record_obj["return_time"] = borrow_record->getReturnDate().empty() ? web::json::value::null() : web::json::value::string(borrow_record->getReturnDate());
            borrow_record_obj["status"] = web::json::value::string(borrow_record->getStatus());
            borrow_record_obj["created_at"] = web::json::value::string(borrow_record->getCreatedAt());
            borrow_record_obj["updated_at"] = web::json::value::string(borrow_record->getUpdatedAt());
            borrow_record_array[borrow_record_array.size()] = borrow_record_obj;
        }
        
        data["borrow_records"] = borrow_record_array;
        data["total"] = total;
        data["page"] = page;
        data["page_size"] = page_size;
        
        sendResponse(request, status_codes::OK, 200, "获取用户借阅记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get user borrow records request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void BorrowController::handleGetBookBorrowRecords(http_request request) {
    Logger::info("Received get book borrow records request");
    
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
        
        // 调用借阅服务获取图书借阅记录
        auto borrow_records = borrow_service->getBookBorrowRecords(book_id, status);
        int total = borrow_service->getBorrowRecordCount(-1, book_id, status);
        
        // 构造响应数据
        web::json::value data;
        web::json::value borrow_record_array = web::json::value::array();
        
        for (const auto& borrow_record : borrow_records) {
            web::json::value borrow_record_obj;
            borrow_record_obj["borrow_id"] = borrow_record->getId();
            borrow_record_obj["user_id"] = borrow_record->getUserId();
            borrow_record_obj["book_id"] = borrow_record->getBookId();
            borrow_record_obj["borrow_time"] = web::json::value::string(borrow_record->getBorrowDate());
            borrow_record_obj["due_time"] = web::json::value::string(borrow_record->getDueDate());
            borrow_record_obj["return_time"] = borrow_record->getReturnDate().empty() ? web::json::value::null() : web::json::value::string(borrow_record->getReturnDate());
            borrow_record_obj["status"] = web::json::value::string(borrow_record->getStatus());
            borrow_record_obj["created_at"] = web::json::value::string(borrow_record->getCreatedAt());
            borrow_record_obj["updated_at"] = web::json::value::string(borrow_record->getUpdatedAt());
            borrow_record_array[borrow_record_array.size()] = borrow_record_obj;
        }
        
        data["borrow_records"] = borrow_record_array;
        data["total"] = total;
        data["page"] = page;
        data["page_size"] = page_size;
        
        sendResponse(request, status_codes::OK, 200, "获取图书借阅记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get book borrow records request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void BorrowController::handleGetOverdueBorrowRecords(http_request request) {
    Logger::info("Received get overdue borrow records request");
    
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
                }
            }
        }
        
        // 调用借阅服务获取逾期借阅记录
        auto overdue_borrow_records = borrow_service->getOverdueBorrowRecords(page, page_size);
        int total = borrow_service->getBorrowRecordCount(-1, -1, "overdue");
        
        // 构造响应数据
        web::json::value data;
        web::json::value overdue_borrow_record_array = web::json::value::array();
        
        for (const auto& overdue_borrow_record : overdue_borrow_records) {
            web::json::value overdue_borrow_record_obj;
            overdue_borrow_record_obj["borrow_id"] = overdue_borrow_record->getId();
            overdue_borrow_record_obj["user_id"] = overdue_borrow_record->getUserId();
            overdue_borrow_record_obj["book_id"] = overdue_borrow_record->getBookId();
            overdue_borrow_record_obj["borrow_time"] = web::json::value::string(overdue_borrow_record->getBorrowDate());
            overdue_borrow_record_obj["due_time"] = web::json::value::string(overdue_borrow_record->getDueDate());
            overdue_borrow_record_obj["return_time"] = overdue_borrow_record->getReturnDate().empty() ? web::json::value::null() : web::json::value::string(overdue_borrow_record->getReturnDate());
            overdue_borrow_record_obj["status"] = web::json::value::string(overdue_borrow_record->getStatus());
            overdue_borrow_record_obj["created_at"] = web::json::value::string(overdue_borrow_record->getCreatedAt());
            overdue_borrow_record_obj["updated_at"] = web::json::value::string(overdue_borrow_record->getUpdatedAt());
            overdue_borrow_record_array[overdue_borrow_record_array.size()] = overdue_borrow_record_obj;
        }
        
        data["overdue_borrow_records"] = overdue_borrow_record_array;
        data["total"] = total;
        data["page"] = page;
        data["page_size"] = page_size;
        
        sendResponse(request, status_codes::OK, 200, "获取逾期借阅记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling get overdue borrow records request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

void BorrowController::handleScanOverdueBorrowRecords(http_request request) {
    Logger::info("Received scan overdue borrow records request");
    
    // 验证用户身份（需要管理员权限）
    int user_id;
    string role;
    if (!authenticateUser(request, user_id, role) || role != "admin") {
        sendResponse(request, status_codes::Unauthorized, 401, "未授权访问，需要管理员权限");
        return;
    }
    
    try {
        // 调用借阅服务扫描逾期借阅记录
        auto overdue_borrow_records = borrow_service->scanOverdueBorrowRecords();
        
        // 构造响应数据
        web::json::value data;
        web::json::value overdue_borrow_record_array = web::json::value::array();
        
        for (const auto& overdue_borrow_record : overdue_borrow_records) {
            web::json::value overdue_borrow_record_obj;
            overdue_borrow_record_obj["borrow_id"] = overdue_borrow_record->getId();
            overdue_borrow_record_obj["user_id"] = overdue_borrow_record->getUserId();
            overdue_borrow_record_obj["book_id"] = overdue_borrow_record->getBookId();
            overdue_borrow_record_obj["borrow_time"] = web::json::value::string(overdue_borrow_record->getBorrowDate());
            overdue_borrow_record_obj["due_time"] = web::json::value::string(overdue_borrow_record->getDueDate());
            overdue_borrow_record_obj["return_time"] = overdue_borrow_record->getReturnDate().empty() ? web::json::value::null() : web::json::value::string(overdue_borrow_record->getReturnDate());
            overdue_borrow_record_obj["status"] = web::json::value::string(overdue_borrow_record->getStatus());
            overdue_borrow_record_obj["created_at"] = web::json::value::string(overdue_borrow_record->getCreatedAt());
            overdue_borrow_record_obj["updated_at"] = web::json::value::string(overdue_borrow_record->getUpdatedAt());
            overdue_borrow_record_array[overdue_borrow_record_array.size()] = overdue_borrow_record_obj;
        }
        
        data["overdue_borrow_records"] = overdue_borrow_record_array;
        
        sendResponse(request, status_codes::OK, 200, "扫描逾期借阅记录成功", data);
        
    } catch (const exception& e) {
        Logger::error("Error handling scan overdue borrow records request: " + string(e.what()));
        sendResponse(request, status_codes::InternalError, 500, "服务器内部错误");
    }
}

bool BorrowController::authenticateUser(http_request request, int& user_id, string& role) {
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
        user_id = user_service.verifyJWTToken(token);
        if (user_id == -1) {
            Logger::error("Invalid JWT Token");
            return false;
        }
        // 获取用户角色
        auto user = user_service.getUserInfo(user_id);
        if (!user) {
            Logger::error("User not found");
            return false;
        }
        role = user->getRole();
        
        return true;
        
    } catch (const exception& e) {
        Logger::error("Error authenticating user: " + string(e.what()));
        return false;
    }
}

void BorrowController::sendResponse(http_request request, http::status_code status, int code, const string& message, const web::json::value& data) {
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