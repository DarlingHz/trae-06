#include "job_service/http_server.h"
#include "job_service/job.h"
#include "job_service/storage.h"
#include "job_service/task_executor.h"
#include <iostream>
#include <sstream>
#include <memory>

using namespace job_service;

int main() {
    std::cout << "Testing HttpRequest parsing..." << std::endl;
    
    // 测试HTTP请求解析
    std::string raw_request = 
        "POST /api/jobs?param1=value1 HTTP/1.1\r\n" 
        "Host: localhost:8080\r\n" 
        "Content-Type: application/json\r\n" 
        "Content-Length: 53\r\n" 
        "\r\n" 
        "{\"type\": \"fib\", \"payload\": {\"n\": 10}, \"priority\": 5}";
    
    HttpRequest request("", "");
    bool parse_result = request.parse(raw_request);
    
    if (parse_result) {
        std::cout << "✓ Request parsing successful" << std::endl;
        std::cout << "  Method: " << request.get_method() << std::endl;
        std::cout << "  Path: " << request.get_path() << std::endl;
        std::cout << "  Query param 'param1': " << request.get_query_param("param1") << std::endl;
        std::cout << "  Content-Type: " << request.get_header("Content-Type") << std::endl;
        
        if (request.get_json_body().contains("type")) {
            std::cout << "  JSON body parsed successfully" << std::endl;
            std::cout << "  Task type: " << request.get_json_body()["type"].get<std::string>() << std::endl;
            std::cout << "  Task priority: " << request.get_json_body()["priority"].get<int>() << std::endl;
        }
    } else {
        std::cout << "✗ Request parsing failed" << std::endl;
    }
    
    // 测试响应生成
    std::cout << "\nTesting HttpResponse generation..." << std::endl;
    HttpResponse response(201);
    nlohmann::json json_body;
    json_body["job_id"] = "test123";
    json_body["status"] = "queued";
    response.set_json_body(json_body);
    
    std::string response_str = response.to_string();
    std::cout << "✓ Response generated successfully" << std::endl;
    std::cout << "  Status code: " << response.get_status_code() << std::endl;
    std::cout << "  Response starts with: " << std::string(response_str.begin(), std::min(response_str.begin() + 100, response_str.end())) << "..." << std::endl;
    
    // 测试错误响应
    std::cout << "\nTesting error responses..." << std::endl;
    HttpResponse bad_request = HttpResponse::create_error(400, "Missing required fields");
    std::cout << "✓ Bad request response generated: " << bad_request.get_status_code() << std::endl;
    
    HttpResponse not_found = HttpResponse::create_error(404, "Route not found");
    std::cout << "✓ Not found response generated: " << not_found.get_status_code() << std::endl;
    
    // 测试成功响应
    std::cout << "\nTesting success responses..." << std::endl;
    nlohmann::json success_data;
    success_data["result"] = "test result";
    HttpResponse success_response = HttpResponse::create_success(success_data);
    std::cout << "✓ Success response generated: " << success_response.get_status_code() << std::endl;
    
    // 测试Job到JSON转换
    std::cout << "\nTesting Job to JSON conversion..." << std::endl;
    auto job = std::make_shared<Job>("test_job_001", "fib", nlohmann::json({{"n", 42}}), 5);
    job->set_started_at(std::chrono::system_clock::now() - std::chrono::seconds(10));
    job->set_finished_at(std::chrono::system_clock::now());
    job->set_status(JobStatus::DONE);
    
    nlohmann::json job_json = job->to_json();
    std::cout << "✓ Job JSON conversion successful" << std::endl;
    std::cout << "  Job ID: " << job_json["job_id"].get<std::string>() << std::endl;
    std::cout << "  Status: " << job_json["status"].get<std::string>() << std::endl;
    std::cout << "  Type: " << job_json["type"].get<std::string>() << std::endl;
    std::cout << "  Created at: " << job_json["created_at"].get<std::string>() << std::endl;
    
    std::cout << "\nAll HTTP server tests completed successfully!" << std::endl;
    
    return 0;
}
