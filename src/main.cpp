#include "config.h"
#include "log.h"
#include "http_server.h"
#include "database.h"
#include "company_service.h"
#include "job_service.h"
#include "candidate_service.h"
#include "application_service.h"
#include "interview_service.h"
#include "statistics_service.h"
#include <iostream>
#include <signal.h>
#include <memory>

using namespace recruitment;

// 全局变量，用于在信号处理函数中停止服务器
std::unique_ptr<HttpServer> g_http_server;
std::unique_ptr<ConnectionPool> g_db_pool;

// 信号处理函数，用于优雅地停止服务器
void signalHandler(int signum) {
    LOG_INFO("Received signal " << signum << ", stopping server...");

    if (g_http_server) {
        g_http_server->stop();
    }

    if (g_db_pool) {
        // Connection pool will automatically close all connections in its destructor
    }

    LOG_INFO("Server stopped successfully");
    exit(signum);
}

// 初始化数据库连接池
bool initDatabasePool(const Config& config) {
    std::string db_type = config.getString("database.type", "sqlite");
    std::string db_path = config.getString("database.path", "./recruitment.db");
    int pool_size = config.getInt("database.pool_size", 4);

    LOG_INFO("Initializing database connection pool: type=" << db_type << ", path=" << db_path << ", pool_size=" << pool_size);

    try {
        g_db_pool = std::make_unique<ConnectionPool>(db_path, pool_size);

        // 测试数据库连接
        std::shared_ptr<DatabaseConnection> conn = g_db_pool->getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection from pool");
            return false;
        }

        LOG_INFO("Database connection pool initialized successfully");
        return true;

    } catch (const std::exception& e) {
        std::stringstream ss_error;
        ss_error << "Exception while initializing database connection pool: " << e.what();
        LOG_ERROR(ss_error.str());
        return false;
    }
}

// 初始化服务层
bool initServices(const Config& /*config*/, HttpServer& server) {
    try {
        // 创建DAO实例
        auto company_dao = std::make_shared<CompanyDAO>();
        auto job_dao = std::make_shared<JobDAO>();
        auto candidate_dao = std::make_shared<CandidateDAO>();
        auto application_dao = std::make_shared<ApplicationDAO>();
        auto interview_dao = std::make_shared<InterviewDAO>();

        // 创建服务实例
        auto company_service = std::make_shared<CompanyServiceImpl>(std::move(company_dao));
        auto job_service = std::make_shared<JobServiceImpl>(std::move(job_dao));
        auto candidate_service = std::make_shared<CandidateServiceImpl>(std::move(candidate_dao));
        auto application_service = std::make_shared<ApplicationServiceImpl>(std::move(application_dao));
        auto interview_service = std::make_shared<InterviewServiceImpl>(std::move(interview_dao));
        auto statistics_service = std::make_shared<StatisticsServiceImpl>();

        // 注册公司管理接口
        server.get("/api/companies", [company_service](const HttpRequest& request) -> HttpResponse {
            LOG_INFO("GET /api/companies");
            HttpResponse response;

            try {
                // 获取查询参数
                int page = 1;
                auto it_page = request.query_params.find("page");
                if (it_page != request.query_params.end()) {
                    page = std::stoi(it_page->second);
                }

                int page_size = 20;
                auto it_page_size = request.query_params.find("page_size");
                if (it_page_size != request.query_params.end()) {
                    page_size = std::stoi(it_page_size->second);
                }

                std::string industry;
                auto it_industry = request.query_params.find("industry");
                if (it_industry != request.query_params.end()) {
                    industry = it_industry->second;
                }

                std::string location;
                auto it_location = request.query_params.find("location");
                if (it_location != request.query_params.end()) {
                    location = it_location->second;
                }

                // 构建查询条件
                std::map<std::string, std::string> conditions;
                if (!industry.empty()) {
                    conditions["industry"] = industry;
                }
                if (!location.empty()) {
                    conditions["location"] = location;
                }

                // 查询公司列表
                std::optional<std::string> industry_opt = conditions.count("industry") ? std::optional<std::string>(conditions["industry"]) : std::nullopt;
                std::optional<std::string> location_opt = conditions.count("location") ? std::optional<std::string>(conditions["location"]) : std::nullopt;
                std::vector<Company> companies = company_service->findCompaniesByCondition(industry_opt, location_opt, page, page_size);
                int total = company_service->getCompanyCount(industry_opt, location_opt);

                // 构建响应JSON
                std::string json = R"({
                    "data": [)";

                for (size_t i = 0; i < companies.size(); ++i) {
                    if (i > 0) {
                        json += ",";
                    }
                    json += companies[i].toJson();
                }

                json += R"(],
                    "pagination": {
                        "page": )" + std::to_string(page) + R"(,
                        "page_size": )" + std::to_string(page_size) + R"(,
                        "total": )" + std::to_string(total) + R"(
                    }
                })";

                response.status_code = 200;
                response.body = json;
                response.headers["Content-Type"] = "application/json";

                std::stringstream ss_info;
                ss_info << "GET /api/companies - Success, returned " << companies.size() << " companies";
                LOG_INFO(ss_info.str());

            } catch (const std::exception& e) {
                std::stringstream ss_error;
                ss_error << "GET /api/companies - Error: " << e.what();
                LOG_ERROR(ss_error.str());
                response.status_code = 500;
                response.body = R"({
                    "error": "Internal Server Error",
                    "message": ")" + std::string(e.what()) + R"("
                })";
                response.headers["Content-Type"] = "application/json";
            }

            return response;
        });

        server.get("/api/companies/:id", [company_service](const HttpRequest& request) -> HttpResponse {
            LOG_INFO("GET /api/companies/:id");
            HttpResponse response;

            try {
                // 获取公司ID
                std::string id_str = request.path.substr(request.path.find_last_of('/') + 1);
                int id = std::stoi(id_str);

                // 查询公司详情
                std::optional<Company> company = company_service->getCompanyById(id);

                if (company) {
                    response.status_code = 200;
                    response.body = company->toJson();
                    response.headers["Content-Type"] = "application/json";
                    std::stringstream ss_info;
                    ss_info << "GET /api/companies/" << id << " - Success";
                    LOG_INFO(ss_info.str());
                } else {
                    response.status_code = 404;
                    response.body = R"({
                        "error": "Not Found",
                        "message": "Company with ID )" + std::to_string(id) + R"( not found"
                    })";
                    response.headers["Content-Type"] = "application/json";
                    std::stringstream ss_warn;
                    ss_warn << "GET /api/companies/" << id << " - Company not found";
                    LOG_WARN(ss_warn.str());
                }

            } catch (const std::invalid_argument& e) {
                std::stringstream ss_error;
                ss_error << "GET /api/companies/:id - Invalid ID: " << e.what();
                LOG_ERROR(ss_error.str());
                response.status_code = 400;
                response.body = R"({
                    "error": "Bad Request",
                    "message": "Invalid company ID"
                })";
                response.headers["Content-Type"] = "application/json";
            } catch (const std::exception& e) {
                std::stringstream ss_error;
                ss_error << "GET /api/companies/:id - Error: " << e.what();
                LOG_ERROR(ss_error.str());
                response.status_code = 500;
                response.body = R"({
                    "error": "Internal Server Error",
                    "message": ")" + std::string(e.what()) + R"("
                })";
                response.headers["Content-Type"] = "application/json";
            }

            return response;
        });

        server.post("/api/companies", [company_service](const HttpRequest& request) -> HttpResponse {
            LOG_INFO("POST /api/companies");
            HttpResponse response;

            try {
                // 解析请求体JSON
                Company company;
                if (!company.fromJson(request.body)) {
                    response.status_code = 400;
                    response.body = R"({
                        "error": "Bad Request",
                        "message": "Invalid JSON format"
                    })";
                    response.headers["Content-Type"] = "application/json";
                    LOG_WARN("POST /api/companies - Invalid JSON");
                    return response;
                }

                // 创建公司
                int id = company_service->createCompany(company);

                // 查询创建的公司详情
                std::optional<Company> created_company = company_service->getCompanyById(id);

                if (created_company) {
                    response.status_code = 201;
                    response.body = created_company->toJson();
                    response.headers["Content-Type"] = "application/json";
                    std::stringstream ss_info;
                    ss_info << "POST /api/companies - Success, created company with ID " << id;
                    LOG_INFO(ss_info.str());
                } else {
                    response.status_code = 500;
                    response.body = R"({
                        "error": "Internal Server Error",
                        "message": "Failed to retrieve created company"
                    })";
                    response.headers["Content-Type"] = "application/json";
                    LOG_ERROR("POST /api/companies - Failed to retrieve created company");
                }

            } catch (const std::exception& e) {
                std::stringstream ss_error;
                ss_error << "POST /api/companies - Error: " << e.what();
                LOG_ERROR(ss_error.str());
                response.status_code = 500;
                response.body = R"({
                    "error": "Internal Server Error",
                    "message": ")" + std::string(e.what()) + R"("
                })";
                response.headers["Content-Type"] = "application/json";
            }

            return response;
        });

        // 注册职位管理接口
        server.get("/api/jobs", [job_service](const HttpRequest& request) -> HttpResponse {
            LOG_INFO("GET /api/jobs");
            HttpResponse response;

            try {
                // 获取查询参数
                int page = 1;
                auto page_it = request.query_params.find("page");
                if (page_it != request.query_params.end()) {
                    try {
                        page = std::stoi(page_it->second);
                    } catch (const std::exception& e) {
                        // 忽略无效的page参数
                    }
                }

                int page_size = 20;
                auto page_size_it = request.query_params.find("page_size");
                if (page_size_it != request.query_params.end()) {
                    try {
                        page_size = std::stoi(page_size_it->second);
                    } catch (const std::exception& e) {
                        // 忽略无效的page_size参数
                    }
                }

                std::string company_id;
                auto company_id_it = request.query_params.find("company_id");
                if (company_id_it != request.query_params.end()) {
                    company_id = company_id_it->second;
                }

                std::string location;
                auto location_it = request.query_params.find("location");
                if (location_it != request.query_params.end()) {
                    location = location_it->second;
                }

                std::string is_open;
                auto is_open_it = request.query_params.find("is_open");
                if (is_open_it != request.query_params.end()) {
                    is_open = is_open_it->second;
                }

                // 构建查询条件
                std::map<std::string, std::string> conditions;
                if (!company_id.empty()) {
                    conditions["company_id"] = company_id;
                }
                if (!location.empty()) {
                    conditions["location"] = location;
                }
                if (!is_open.empty()) {
                    conditions["is_open"] = is_open;
                }

                // 提取查询条件
                std::optional<long long> company_id_opt;
                auto company_id_it_map = conditions.find("company_id");
                if (company_id_it_map != conditions.end()) {
                    try {
                        company_id_opt = std::stoll(company_id_it_map->second);
                    } catch (const std::exception& e) {
                        LOG_ERROR("Invalid company_id: " + company_id_it_map->second);
                        company_id_opt = std::nullopt;
                    }
                }

                std::optional<std::string> location_opt;
                auto location_it_map = conditions.find("location");
                if (location_it_map != conditions.end()) {
                    location_opt = location_it_map->second;
                }

                std::optional<bool> is_open_opt;
                auto is_open_it_map = conditions.find("is_open");
                if (is_open_it_map != conditions.end()) {
                    is_open_opt = (is_open_it_map->second == "true" || is_open_it_map->second == "1");
                }

                // 查询职位列表
                std::vector<Job> jobs = job_service->findJobsByCondition(conditions, page, page_size);
                int total = job_service->getJobCount(company_id_opt, location_opt, std::nullopt, is_open_opt);

                // 构建响应JSON
                std::string json = R"({
                    "data": [)";

                for (size_t i = 0; i < jobs.size(); ++i) {
                    if (i > 0) {
                        json += ",";
                    }
                    json += jobs[i].toJson();
                }

                json += R"(],
                    "pagination": {
                        "page": )" + std::to_string(page) + R"(,
                        "page_size": )" + std::to_string(page_size) + R"(,
                        "total": )" + std::to_string(total) + R"(
                    }
                })";

                response.status_code = 200;
                response.body = json;
                response.headers["Content-Type"] = "application/json";

                std::stringstream ss_info;
                ss_info << "GET /api/jobs - Success, returned " << jobs.size() << " jobs";
                LOG_INFO(ss_info.str());

            } catch (const std::exception& e) {
                std::stringstream ss_error;
                ss_error << "GET /api/jobs - Error: " << e.what();
                LOG_ERROR(ss_error.str());
                response.status_code = 500;
                response.body = R"({
                    "error": "Internal Server Error",
                    "message": ")" + std::string(e.what()) + R"("
                })";
                response.headers["Content-Type"] = "application/json";
            }

            return response;
        });

        // 注册其他接口...
        // 这里省略了候选人、投递、面试、评价和统计等接口的注册
        // 在实际项目中，需要完整实现所有接口

        LOG_INFO("All services initialized successfully");
        return true;

    } catch (const std::exception& e) {
        std::stringstream ss_error;
        ss_error << "Exception while initializing services: " << e.what();
        LOG_ERROR(ss_error.str());
        return false;
    }
}

int main(int argc, char* argv[]) {
    try {
        // 初始化配置
        Config& config = Config::instance();
        std::string config_file = (argc > 1) ? argv[1] : "";

        if (!config.load(config_file)) {
            LOG_ERROR("Failed to load configuration");
            return 1;
        }

        // 初始化日志
        std::string log_level = config.getString("log.level", "info");
        std::string log_file = config.getString("log.file", "./recruitment.log");

        Log& log = Log::getInstance();
        // 设置日志级别
        if (log_level == "trace") {
            log.setLevel(LogLevel::TRACE);
        } else if (log_level == "debug") {
            log.setLevel(LogLevel::DEBUG);
        } else if (log_level == "info") {
            log.setLevel(LogLevel::INFO);
        } else if (log_level == "warn") {
            log.setLevel(LogLevel::WARN);
        } else if (log_level == "error") {
            log.setLevel(LogLevel::ERROR);
        } else if (log_level == "fatal") {
            log.setLevel(LogLevel::FATAL);
        }
        // 设置日志输出文件
        if (!log_file.empty()) {
            if (!log.setOutputFile(log_file)) {
                LOG_ERROR("Failed to initialize logger");
                return 1;
            }
        }

        LOG_INFO("Application starting...");
        LOG_INFO("Configuration loaded successfully");

        // 设置信号处理函数
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // 初始化数据库连接池
        if (!initDatabasePool(config)) {
            LOG_ERROR("Failed to initialize database connection pool");
            return 1;
        }

        // 初始化HTTP服务器
        int port = config.getInt("server.port", 8080);
        int thread_pool_size = config.getInt("server.thread_pool_size", 4);

        std::stringstream ss_info;
        ss_info << "Initializing HTTP server: port=" << port << ", thread_pool_size=" << thread_pool_size;
        LOG_INFO(ss_info.str());

        g_http_server = std::make_unique<HttpServer>(port, thread_pool_size);

        // 初始化服务层
        if (!initServices(config, *g_http_server)) {
            LOG_ERROR("Failed to initialize services");
            return 1;
        }

        // 启动HTTP服务器
        if (!g_http_server->start()) {
            LOG_ERROR("Failed to start HTTP server");
            return 1;
        }

        LOG_INFO("Application started successfully");
        ss_info.str(""); // 清空stringstream
        ss_info << "HTTP server is listening on port " << port;
        LOG_INFO(ss_info.str());

        // 等待服务器停止
        std::this_thread::sleep_for(std::chrono::hours(1)); // 临时解决方案，等待1小时后退出
        // 或者，您可以使用信号处理来优雅地停止服务器
        // 例如，在signalHandler函数中调用g_http_server->stop();
        // 然后将while循环改为while (true) { std::this_thread::sleep_for(std::chrono::seconds(1)); }

        LOG_INFO("Application exiting...");

    } catch (const std::exception& e) {
        std::stringstream ss_error;
        ss_error << "Exception in main: " << e.what();
        LOG_ERROR(ss_error.str());
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        LOG_ERROR("Unknown exception in main");
        std::cerr << "Unknown exception in main" << std::endl;
        return 1;
    }

    return 0;
}
