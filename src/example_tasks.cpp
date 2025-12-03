#include "job_service/task_factory.h"
#include "job_service/job.h"
#include "job_service/utils.h"
#include <string>
#include <sstream>
#include <vector>
#include <map>

namespace job_service {

// 斐波那契数列计算任务
class FibonacciTask : public Task {
public:
    FibonacciTask() = default;
    
    std::string get_type() const override {
        return "fib";
    }
    
    nlohmann::json execute(JobPtr job) override {
        if (!job) {
            throw std::invalid_argument("Job pointer is null");
        }
        
        const auto& payload = job->get_payload();
        
        if (!payload.contains("n") || !payload["n"].is_number_integer()) {
            throw std::invalid_argument("Invalid input: 'n' must be an integer");
        }
        
        int n = payload["n"].get<int>();
        
        global_logger.info("Executing fibonacci task for n=", n);
        
        long long result = 0;
        if (n <= 1) {
            result = n;
        } else {
            long long a = 0, b = 1;
            for (int i = 2; i <= n; ++i) {
                long long temp = b;
                b = a + b;
                a = temp;
            }
            result = b;
        }
        
        nlohmann::json result_json;
        global_logger.info("Fibonacci task completed with result=", result);
        
        result_json["n"] = n;
        result_json["fibonacci"] = result;
        
        return result_json;
    }
    
    std::unique_ptr<Task> clone() const override {
        return std::make_unique<FibonacciTask>();
    }
};

// 单词计数任务
class WordCountTask : public Task {
public:
    WordCountTask() = default;
    
    std::string get_type() const override {
        return "word_count";
    }
    
    nlohmann::json execute(JobPtr job) override {
        if (!job) {
            throw std::invalid_argument("Job pointer is null");
        }
        
        const auto& payload = job->get_payload();
        
        if (!payload.contains("text") || !payload["text"].is_string()) {
            throw std::invalid_argument("Invalid input: 'text' must be a string");
        }
        
        std::string text = payload["text"];
        
        int count = 0;
        bool in_word = false;
        for (char c : text) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                in_word = false;
            } else if (!in_word) {
                in_word = true;
                count++;
            }
        }
        
        nlohmann::json result_json;
        result_json["word_count"] = count;
        result_json["text_length"] = text.size();
        
        return result_json;
    }
    
    std::unique_ptr<Task> clone() const override {
        return std::make_unique<WordCountTask>();
    }
};

// 任务类型注册函数
void register_example_tasks(std::shared_ptr<TaskFactory> factory) {
    if (!factory) {
        return;
    }
    
    factory->register_task_type("fib", []() { return std::make_unique<FibonacciTask>(); });
    factory->register_task_type("word_count", []() { return std::make_unique<WordCountTask>(); });
}

} // namespace job_service
