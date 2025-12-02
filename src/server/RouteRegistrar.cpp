#include "server/RouteRegistrar.h"
#include "service/UserService.h"
#include "service/PetService.h"
#include "service/DoctorService.h"
#include "service/AppointmentService.h"
#include "service/RecordService.h"
#include "logging/Logging.h"
#include "config/Config.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace pet_hospital {

RouteRegistrar::RouteRegistrar(HTTPServer& server) : server_(server) {
}

RouteRegistrar::~RouteRegistrar() {
}

void RouteRegistrar::register_all_routes() {
    register_health_check_routes();
    register_user_routes();
    register_pet_routes();
    register_doctor_routes();
    register_appointment_routes();
    register_record_routes();

    LOG_INFO("All routes registered successfully");
}

void RouteRegistrar::register_user_routes() {
    // 用户注册
    server_.register_route("POST", "/api/users/register", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 解析请求体
            json request_body = json::parse(request.body);

            // 验证请求参数
            if (!request_body.contains("email") || !request_body.contains("password") || !request_body.contains("name")) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing required parameters\"}";
                return response;
            }

            std::string email = request_body["email"];
            std::string password = request_body["password"];
            std::string name = request_body["name"];

            // 调用用户服务注册用户
            UserService user_service;
            std::string error_message;
            auto user = user_service.register_user(email, password, name, error_message);

            if (!user) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"REGISTER_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = user->get_id();
            response_body["email"] = user->get_email();
            response_body["name"] = user->get_name();
            response_body["created_at"] = user->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse request body: " + std::string(e.what()));
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"code\": \"INVALID_JSON\", \"message\": \"Invalid JSON format\"}";
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to register user: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 用户登录
    server_.register_route("POST", "/api/users/login", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 解析请求体
            json request_body = json::parse(request.body);

            // 验证请求参数
            if (!request_body.contains("email") || !request_body.contains("password")) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing required parameters\"}";
                return response;
            }

            std::string email = request_body["email"];
            std::string password = request_body["password"];

            // 调用用户服务登录
            UserService user_service;
            std::string error_message;
            auto token = user_service.login_user(email, password, error_message);

            if (!token) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"LOGIN_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 获取登录用户的信息
            auto user = user_service.get_user_info(token->get_user_id(), error_message);
            if (!user) {
                response.status_code = 500;
                response.status_message = "Internal Server Error";
                response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Failed to retrieve user info\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["token"] = token->get_token();
            response_body["expires_at"] = token->get_expires_at();
            json user_json;
            user_json["id"] = user->get_id();
            user_json["email"] = user->get_email();
            user_json["name"] = user->get_name();
            response_body["user"] = user_json;

            response.body = response_body.dump();
            return response;
        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse request body: " + std::string(e.what()));
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"code\": \"INVALID_JSON\", \"message\": \"Invalid JSON format\"}";
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to login user: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取用户信息
    server_.register_route("GET", "/api/users/profile", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = user->get_id();
            response_body["email"] = user->get_email();
            response_body["name"] = user->get_name();
            response_body["created_at"] = user->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get user profile: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 更新用户信息
    server_.register_route("PUT", "/api/users/profile", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析请求体
            json request_body = json::parse(request.body);

            // 提取更新参数
            std::optional<std::string> name;
            if (request_body.contains("name")) {
                name = request_body["name"];
            }

            // 调用用户服务更新用户信息
            if (!user_service.update_user_info(user->get_id(), name.value_or(""), "", error_message)) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"UPDATE_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 获取更新后的用户信息
            auto updated_user = user_service.get_user_info(user->get_id(), error_message);
            if (!updated_user) {
                response.status_code = 500;
                response.status_message = "Internal Server Error";
                response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Failed to retrieve updated user\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = updated_user->get_id();
            response_body["email"] = updated_user->get_email();
            response_body["name"] = updated_user->get_name();
            response_body["created_at"] = updated_user->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse request body: " + std::string(e.what()));
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"code\": \"INVALID_JSON\", \"message\": \"Invalid JSON format\"}";
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to update user profile: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    LOG_INFO("User routes registered successfully");
}

void RouteRegistrar::register_pet_routes() {
    // 添加宠物
    server_.register_route("POST", "/api/pets", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析请求体
            json request_body = json::parse(request.body);

            // 验证请求参数
            if (!request_body.contains("name") || !request_body.contains("species") || !request_body.contains("breed") || !request_body.contains("gender")) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing required parameters\"}";
                return response;
            }

            std::string name = request_body["name"];
            std::string species = request_body["species"];
            std::string breed = request_body["breed"];
            std::string gender_str = request_body["gender"];
            Pet::Gender gender = Pet::Gender::UNKNOWN;
            if (gender_str == "male" || gender_str == "MALE") {
                gender = Pet::Gender::MALE;
            } else if (gender_str == "female" || gender_str == "FEMALE") {
                gender = Pet::Gender::FEMALE;
            }
            std::optional<std::string> birthday;
            if (request_body.contains("birthday")) {
                birthday = request_body["birthday"];
            }
            std::optional<std::string> notes;
            if (request_body.contains("notes")) {
                notes = request_body["notes"];
            }

            std::optional<double> weight;
            if (request_body.contains("weight")) {
                weight = request_body["weight"];
            }

            // 调用宠物服务添加宠物
            PetService pet_service;
            auto pet = pet_service.add_pet(user->get_id(), name, species, breed, gender, birthday, weight, notes, error_message);

            if (!pet) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"ADD_PET_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = pet->get_id();
            response_body["name"] = pet->get_name();
            response_body["species"] = pet->get_species();
            response_body["breed"] = pet->get_breed();
            response_body["gender"] = pet->get_gender();
            if (pet->get_birthday()) {
                response_body["birthday"] = pet->get_birthday().value();
            }
            if (pet->get_notes()) {
                response_body["notes"] = pet->get_notes().value();
            }
            response_body["created_at"] = pet->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse request body: " + std::string(e.what()));
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"code\": \"INVALID_JSON\", \"message\": \"Invalid JSON format\"}";
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to add pet: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取宠物列表
    server_.register_route("GET", "/api/pets", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析分页参数
            int page = 1;
            int page_size = 10;

            // 解析查询参数
            std::unordered_map<std::string, std::string> query_params;
            if (!request.query.empty()) {
                std::string query = request.query;
                size_t pos = 0;
                while (pos < query.size()) {
                    size_t ampersand_pos = query.find('&', pos);
                    std::string param = query.substr(pos, ampersand_pos - pos);
                    size_t equal_pos = param.find('=');
                    if (equal_pos != std::string::npos) {
                        std::string key = param.substr(0, equal_pos);
                        std::string value = param.substr(equal_pos + 1);
                        query_params[key] = value;
                    }
                    pos = ampersand_pos != std::string::npos ? ampersand_pos + 1 : query.size();
                }
            }

            if (query_params.find("page") != query_params.end()) {
                page = std::stoi(query_params["page"]);
            }
            if (query_params.find("page_size") != query_params.end()) {
                page_size = std::stoi(query_params["page_size"]);
            }

            // 调用宠物服务获取宠物列表
            PetService pet_service;
            auto pets = pet_service.get_pets(user->get_id(), error_message, page, page_size);

            if (pets.empty() && !error_message.empty()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"GET_PETS_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["pets"] = json::array();
            for (const auto& pet : pets) {
                json pet_json;
                pet_json["id"] = pet.get_id();
                pet_json["name"] = pet.get_name();
                pet_json["species"] = pet.get_species();
                pet_json["breed"] = pet.get_breed();
                pet_json["gender"] = pet.get_gender();
                if (pet.get_birthday()) {
                    pet_json["birthday"] = pet.get_birthday().value();
                }
                if (pet.get_notes()) {
                    pet_json["notes"] = pet.get_notes().value();
                }
                pet_json["created_at"] = pet.get_created_at();
                response_body["pets"].push_back(pet_json);
            }
            response_body["page"] = page;
            response_body["page_size"] = page_size;
            response_body["total"] = pets.size();

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get pets: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取宠物详情
    server_.register_route("GET", "/api/pets/:id", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析宠物ID
            auto pet_id_param = request.params.find("id");
            if (pet_id_param == request.params.end()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing pet ID\"}";
                return response;
            }

            int pet_id = std::stoi(pet_id_param->second);

            // 调用宠物服务获取宠物详情
            PetService pet_service;
            auto pet = pet_service.get_pet_by_id(pet_id, error_message);

            if (!pet) {
                response.status_code = 404;
                response.status_message = "Not Found";
                response.body = "{\"code\": \"PET_NOT_FOUND\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = pet->get_id();
            response_body["name"] = pet->get_name();
            response_body["species"] = pet->get_species();
            response_body["breed"] = pet->get_breed();
            response_body["gender"] = pet->get_gender();
            if (pet->get_birthday()) {
                response_body["birthday"] = pet->get_birthday().value();
            }
            if (pet->get_notes()) {
                response_body["notes"] = pet->get_notes().value();
            }
            response_body["created_at"] = pet->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get pet: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 更新宠物信息
    server_.register_route("PUT", "/api/pets/:id", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析宠物ID
            auto pet_id_param = request.params.find("id");
            if (pet_id_param == request.params.end()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing pet ID\"}";
                return response;
            }

            int pet_id = std::stoi(pet_id_param->second);

            // 解析请求体
            json request_body = json::parse(request.body);

            // 提取更新参数
            std::string name;
            if (request_body.contains("name")) {
                name = request_body["name"];
            } else {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing name\"}";
                return response;
            }
            std::string species;
            if (request_body.contains("species")) {
                species = request_body["species"];
            } else {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing species\"}";
                return response;
            }
            std::optional<std::string> breed;
            if (request_body.contains("breed")) {
                breed = request_body["breed"];
            }
            Pet::Gender gender = Pet::Gender::UNKNOWN;
            if (request_body.contains("gender")) {
                std::string gender_str = request_body["gender"];
                if (gender_str == "male" || gender_str == "MALE") {
                    gender = Pet::Gender::MALE;
                } else if (gender_str == "female" || gender_str == "FEMALE") {
                    gender = Pet::Gender::FEMALE;
                } else if (gender_str == "unknown" || gender_str == "UNKNOWN") {
                    gender = Pet::Gender::UNKNOWN;
                } else {
                    response.status_code = 400;
                    response.status_message = "Bad Request";
                    response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Invalid gender\"}";
                    return response;
                }
            }
            std::optional<std::string> birthday;
            if (request_body.contains("birthday")) {
                birthday = request_body["birthday"];
            }
            std::optional<double> weight;
            if (request_body.contains("weight")) {
                weight = request_body["weight"];
            }
            std::optional<std::string> notes;
            if (request_body.contains("notes")) {
                notes = request_body["notes"];
            }

            // 调用宠物服务更新宠物信息
            PetService pet_service;
            if (!pet_service.update_pet(pet_id, user->get_id(), name, species, breed, gender, birthday, weight, notes, error_message)) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"UPDATE_PET_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 获取更新后的宠物信息
            auto updated_pet = pet_service.get_pet_by_id(pet_id, error_message);
            if (!updated_pet) {
                response.status_code = 500;
                response.status_message = "Internal Server Error";
                response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Failed to retrieve updated pet\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = updated_pet->get_id();
            response_body["name"] = updated_pet->get_name();
            response_body["species"] = updated_pet->get_species();
            response_body["breed"] = updated_pet->get_breed();
            response_body["gender"] = updated_pet->get_gender();
            if (updated_pet->get_birthday()) {
                response_body["birthday"] = updated_pet->get_birthday().value();
            }
            if (updated_pet->get_notes()) {
                response_body["notes"] = updated_pet->get_notes().value();
            }
            response_body["created_at"] = updated_pet->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse request body: " + std::string(e.what()));
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"code\": \"INVALID_JSON\", \"message\": \"Invalid JSON format\"}";
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to update pet: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 删除宠物
    server_.register_route("DELETE", "/api/pets/:id", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析宠物ID
            auto pet_id_param = request.params.find("id");
            if (pet_id_param == request.params.end()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing pet ID\"}";
                return response;
            }

            int pet_id = std::stoi(pet_id_param->second);

            // 调用宠物服务删除宠物
            PetService pet_service;
            if (!pet_service.delete_pet(pet_id, user->get_id(), error_message)) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"DELETE_PET_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["message"] = "Pet deleted successfully";

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to delete pet: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    LOG_INFO("Pet routes registered successfully");
}

void RouteRegistrar::register_doctor_routes() {
    // 获取医生列表
    server_.register_route("GET", "/api/doctors", [](const HTTPRequest& /*request*/) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 调用医生服务获取医生列表
            DoctorService doctor_service;
            std::string error_message;
            auto doctors = doctor_service.get_all_doctors(error_message);

            if (doctors.empty() && !error_message.empty()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"GET_DOCTORS_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["doctors"] = json::array();
            for (const auto& doctor : doctors) {
                json doctor_json;
                doctor_json["id"] = doctor.get_id();
                doctor_json["name"] = doctor.get_name();
                doctor_json["department_id"] = doctor.get_department_id();
                if (doctor.get_title()) {
                    doctor_json["title"] = doctor.get_title().value();
                }
                if (doctor.get_specialty()) {
                    doctor_json["specialty"] = doctor.get_specialty().value();
                }
                doctor_json["available_start"] = doctor.get_available_start();
                doctor_json["available_end"] = doctor.get_available_end();
                doctor_json["created_at"] = doctor.get_created_at();
                response_body["doctors"].push_back(doctor_json);
            }

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get doctors: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取医生详情
    server_.register_route("GET", "/api/doctors/:id", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 解析医生ID
            auto doctor_id_param = request.params.find("id");
            if (doctor_id_param == request.params.end()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing doctor ID\"}";
                return response;
            }

            int doctor_id = std::stoi(doctor_id_param->second);

            // 调用医生服务获取医生详情
            DoctorService doctor_service;
            std::string error_message;
            auto doctor = doctor_service.get_doctor_by_id(doctor_id, error_message);

            if (!doctor) {
                response.status_code = 404;
                response.status_message = "Not Found";
                response.body = "{\"code\": \"DOCTOR_NOT_FOUND\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = doctor->get_id();
            response_body["name"] = doctor->get_name();
            response_body["department_id"] = doctor->get_department_id();
            if (doctor->get_title()) {
                response_body["title"] = doctor->get_title().value();
            }
            if (doctor->get_specialty()) {
                response_body["specialty"] = doctor->get_specialty().value();
            }
            response_body["available_start"] = doctor->get_available_start();
            response_body["available_end"] = doctor->get_available_end();
            response_body["created_at"] = doctor->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get doctor: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取科室列表
    server_.register_route("GET", "/api/departments", [](const HTTPRequest& /*request*/) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 调用医生服务获取科室列表
            DoctorService doctor_service;
            std::string error_message;
            auto departments = doctor_service.get_all_departments(error_message);

            if (departments.empty() && !error_message.empty()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"GET_DEPARTMENTS_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["departments"] = json::array();
            for (const auto& department : departments) {
                json department_json;
                department_json["id"] = department.get_id();
                department_json["name"] = department.get_name();
                department_json["description"] = department.get_description();
                response_body["departments"].push_back(department_json);
            }

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get departments: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    LOG_INFO("Doctor routes registered successfully");
}

void RouteRegistrar::register_appointment_routes() {
    // 创建预约
    server_.register_route("POST", "/api/appointments", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析请求体
            json request_body = json::parse(request.body);

            // 验证请求参数
            if (!request_body.contains("pet_id") || !request_body.contains("doctor_id") || 
                !request_body.contains("start_time") || !request_body.contains("end_time") || 
                !request_body.contains("reason")) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing required parameters\"}";
                return response;
            }

            int pet_id = request_body["pet_id"];
            int doctor_id = request_body["doctor_id"];
            std::string start_time = request_body["start_time"];
            std::string end_time = request_body["end_time"];
            std::string reason = request_body["reason"];

            // 调用预约服务创建预约
            AppointmentService appointment_service;
            auto appointment = appointment_service.create_appointment(user->get_id(), pet_id, doctor_id, start_time, end_time, reason, error_message);

            if (!appointment) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"CREATE_APPOINTMENT_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = appointment->get_id();
            response_body["user_id"] = appointment->get_user_id();
            response_body["pet_id"] = appointment->get_pet_id();
            response_body["doctor_id"] = appointment->get_doctor_id();
            response_body["start_time"] = appointment->get_start_time();
            response_body["end_time"] = appointment->get_end_time();
            response_body["reason"] = appointment->get_reason();
            response_body["status"] = Appointment::status_to_string(appointment->get_status());
            response_body["created_at"] = appointment->get_created_at();

            response.body = response_body.dump();
            return response;
        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse request body: " + std::string(e.what()));
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"code\": \"INVALID_JSON\", \"message\": \"Invalid JSON format\"}";
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create appointment: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取我的预约
    server_.register_route("GET", "/api/appointments/my", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析时间参数
            std::optional<std::string> from_time;
            std::optional<std::string> to_time;
            
            // 解析查询字符串
            std::map<std::string, std::string> query_params;
            std::string query = request.query;
            size_t pos = 0;
            while (pos < query.size()) {
                size_t amp_pos = query.find('&', pos);
                std::string param = query.substr(pos, amp_pos - pos);
                
                size_t eq_pos = param.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = param.substr(0, eq_pos);
                    std::string value = param.substr(eq_pos + 1);
                    query_params[key] = value;
                }
                
                pos = amp_pos != std::string::npos ? amp_pos + 1 : query.size();
            }
            
            // 获取时间参数
            if (query_params.find("from") != query_params.end()) {
                from_time = query_params["from"];
            }
            if (query_params.find("to") != query_params.end()) {
                to_time = query_params["to"];
            }

            // 调用预约服务获取我的预约
            AppointmentService appointment_service;
            auto appointments = appointment_service.get_appointments_by_user_id(user->get_id(), from_time, to_time, error_message);

            if (appointments.empty() && !error_message.empty()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"GET_APPOINTMENTS_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["appointments"] = json::array();
            for (const auto& appointment : appointments) {
                json appointment_json;
                appointment_json["id"] = appointment.get_id();
                appointment_json["user_id"] = appointment.get_user_id();
                appointment_json["pet_id"] = appointment.get_pet_id();
                appointment_json["doctor_id"] = appointment.get_doctor_id();
                appointment_json["start_time"] = appointment.get_start_time();
                appointment_json["end_time"] = appointment.get_end_time();
                appointment_json["reason"] = appointment.get_reason();
                appointment_json["status"] = Appointment::status_to_string(appointment.get_status());
                appointment_json["created_at"] = appointment.get_created_at();
                response_body["appointments"].push_back(appointment_json);
            }

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get my appointments: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取医生预约
    server_.register_route("GET", "/api/appointments/doctor/:id", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析医生ID
            auto doctor_id_param = request.params.find("id");
            if (doctor_id_param == request.params.end()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing doctor ID\"}";
                return response;
            }

            int doctor_id = std::stoi(doctor_id_param->second);

            // 解析日期参数
            std::optional<std::string> date;
            
            // 解析查询字符串
            std::map<std::string, std::string> query_params;
            std::string query = request.query;
            size_t pos = 0;
            while (pos < query.size()) {
                size_t amp_pos = query.find('&', pos);
                std::string param = query.substr(pos, amp_pos - pos);
                
                size_t eq_pos = param.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = param.substr(0, eq_pos);
                    std::string value = param.substr(eq_pos + 1);
                    query_params[key] = value;
                }
                
                pos = amp_pos != std::string::npos ? amp_pos + 1 : query.size();
            }
            
            // 获取日期参数
            if (query_params.find("date") != query_params.end()) {
                date = query_params["date"];
            }

            // 调用预约服务获取医生预约
            AppointmentService appointment_service;
            auto appointments = appointment_service.get_appointments_by_doctor_id(doctor_id, date, error_message);

            if (appointments.empty() && !error_message.empty()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"GET_APPOINTMENTS_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["appointments"] = json::array();
            for (const auto& appointment : appointments) {
                json appointment_json;
                appointment_json["id"] = appointment.get_id();
                appointment_json["user_id"] = appointment.get_user_id();
                appointment_json["pet_id"] = appointment.get_pet_id();
                appointment_json["doctor_id"] = appointment.get_doctor_id();
                appointment_json["start_time"] = appointment.get_start_time();
                appointment_json["end_time"] = appointment.get_end_time();
                appointment_json["reason"] = appointment.get_reason();
                appointment_json["status"] = Appointment::status_to_string(appointment.get_status());
                appointment_json["created_at"] = appointment.get_created_at();
                response_body["appointments"].push_back(appointment_json);
            }

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get doctor appointments: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 取消预约
    server_.register_route("POST", "/api/appointments/:id/cancel", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析预约ID
            auto appointment_id_param = request.params.find("id");
            if (appointment_id_param == request.params.end()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing appointment ID\"}";
                return response;
            }

            int appointment_id = std::stoi(appointment_id_param->second);

            // 调用预约服务取消预约
            AppointmentService appointment_service;
            if (!appointment_service.cancel_appointment(appointment_id, user->get_id(), error_message)) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"CANCEL_APPOINTMENT_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["message"] = "Appointment canceled successfully";

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to cancel appointment: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    LOG_INFO("Appointment routes registered successfully");
}

void RouteRegistrar::register_record_routes() {
    // 创建病例记录
    server_.register_route("POST", "/api/records", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析请求体
            json request_body = json::parse(request.body);

            // 验证请求参数
            if (!request_body.contains("appointment_id") || !request_body.contains("chief_complaint") || 
                !request_body.contains("diagnosis") || !request_body.contains("treatment")) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing required parameters\"}";
                return response;
            }

            int appointment_id = request_body["appointment_id"];
            std::string chief_complaint = request_body["chief_complaint"];
            std::string diagnosis = request_body["diagnosis"];
            std::string treatment = request_body["treatment"];
            std::optional<std::string> notes;
            if (request_body.contains("notes")) {
                notes = request_body["notes"];
            }

            // 调用病例记录服务创建病例记录
            RecordService record_service;
            auto record = record_service.create_record(appointment_id, chief_complaint, diagnosis, treatment, notes, error_message);

            if (!record) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"CREATE_RECORD_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["id"] = record->get_id();
            response_body["appointment_id"] = record->get_appointment_id();
            response_body["chief_complaint"] = record->get_chief_complaint();
            response_body["diagnosis"] = record->get_diagnosis();
            response_body["treatment"] = record->get_treatment();
            if (record->get_notes()) {
                response_body["notes"] = record->get_notes().value();
            }
            response_body["created_at"] = record->get_created_at();
            response_body["updated_at"] = record->get_updated_at();

            response.body = response_body.dump();
            return response;
        } catch (const json::parse_error& e) {
            LOG_ERROR("Failed to parse request body: " + std::string(e.what()));
            response.status_code = 400;
            response.status_message = "Bad Request";
            response.body = "{\"code\": \"INVALID_JSON\", \"message\": \"Invalid JSON format\"}";
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create record: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    // 获取宠物病例记录
    server_.register_route("GET", "/api/records/pet/:id", [](const HTTPRequest& request) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 验证token
            auto auth_header = request.headers.find("Authorization");
            if (auth_header == request.headers.end()) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"Missing Authorization header\"}";
                return response;
            }

            std::string token = auth_header->second.substr(7); // 去除 "Bearer " 前缀
            UserService user_service;
            std::string error_message;
            auto user = user_service.validate_token(token, error_message);

            if (!user) {
                response.status_code = 401;
                response.status_message = "Unauthorized";
                response.body = "{\"code\": \"AUTH_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 解析宠物ID
            auto pet_id_param = request.params.find("id");
            if (pet_id_param == request.params.end()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"INVALID_PARAM\", \"message\": \"Missing pet ID\"}";
                return response;
            }

            int pet_id = std::stoi(pet_id_param->second);

            // 解析分页参数
            int page = 1;
            int page_size = 10;
            
            // 解析查询字符串
            std::map<std::string, std::string> query_params;
            std::string query = request.query;
            size_t pos = 0;
            while (pos < query.size()) {
                size_t amp_pos = query.find('&', pos);
                std::string param = query.substr(pos, amp_pos - pos);
                
                size_t eq_pos = param.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = param.substr(0, eq_pos);
                    std::string value = param.substr(eq_pos + 1);
                    query_params[key] = value;
                }
                
                pos = amp_pos != std::string::npos ? amp_pos + 1 : query.size();
            }
            
            // 获取分页参数
            if (query_params.find("page") != query_params.end()) {
                page = std::stoi(query_params["page"]);
            }
            if (query_params.find("page_size") != query_params.end()) {
                page_size = std::stoi(query_params["page_size"]);
            }

            // 调用病例记录服务获取宠物病例记录
            RecordService record_service;
            auto records = record_service.get_records_by_pet_id(pet_id, user->get_id(), page, page_size, error_message);

            if (records.empty() && !error_message.empty()) {
                response.status_code = 400;
                response.status_message = "Bad Request";
                response.body = "{\"code\": \"GET_RECORDS_FAILED\", \"message\": \"" + error_message + "\"}";
                return response;
            }

            // 生成响应
            json response_body;
            response_body["records"] = json::array();
            for (const auto& record : records) {
                json record_json;
                record_json["id"] = record.get_id();
                record_json["appointment_id"] = record.get_appointment_id();
                record_json["chief_complaint"] = record.get_chief_complaint();
                record_json["diagnosis"] = record.get_diagnosis();
                record_json["treatment"] = record.get_treatment();
                if (record.get_notes()) {
                    record_json["notes"] = record.get_notes().value();
                }
                record_json["created_at"] = record.get_created_at();
                record_json["updated_at"] = record.get_updated_at();
                response_body["records"].push_back(record_json);
            }
            response_body["page"] = page;
            response_body["page_size"] = page_size;
            response_body["total"] = records.size();

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get pet records: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    LOG_INFO("Record routes registered successfully");
}

void RouteRegistrar::register_health_check_routes() {
    // 健康检查
    server_.register_route("GET", "/health", [](const HTTPRequest& /*request*/) -> HTTPResponse {
        HTTPResponse response;
        try {
            // 生成健康检查响应
            json response_body;
            response_body["status"] = "ok";
            response_body["timestamp"] = std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));

            response.body = response_body.dump();
            return response;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to perform health check: " + std::string(e.what()));
            response.status_code = 500;
            response.status_message = "Internal Server Error";
            response.body = "{\"code\": \"INTERNAL_ERROR\", \"message\": \"Internal server error\"}";
            return response;
        }
    });

    LOG_INFO("Health check routes registered successfully");
}

} // namespace pet_hospital
