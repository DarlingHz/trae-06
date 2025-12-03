#include "http/handler.h"
#include <fmt/format.h>
#include <stdexcept>
#include <chrono>
#include <thread>

namespace http {

using namespace std::chrono;

void ContractHandler::init_routes(httplib::Server& server) {
    // Logging middleware
    server.set_pre_routing_handler([](const auto& req, auto& res) {
        // Start time tracking
        auto start = high_resolution_clock::now();
        
        // Store start time in request context
        req.context.emplace("start_time", start);
        
        return httplib::PreRoutingResult::Normal;
    });
    
    // Logging middleware for responses
    server.set_post_routing_handler([](const auto& req, auto& res) {
        auto it = req.context.find("start_time");
        if (it != req.context.end()) {
            auto start = std::any_cast<high_resolution_clock::time_point>(it->second);
            auto end = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(end - start).count();
            
            fmt::print("{} {} {} {}""ms""\n", req.method, req.path, res.status, duration);
        }
        
        return httplib::PostRoutingResult::Normal;
    });
    
    // API routes
    server.Post("/contracts", [this](const auto& req, auto& res) { handle_create_contract(req, res); });
    server.Put("/contracts/(\\d+)", [this](const auto& req, auto& res) { handle_update_contract(req, res); });
    server.Post("/contracts/(\\d+)/submit", [this](const auto& req, auto& res) { handle_submit_contract(req, res); });
    server.Get("/contracts/(\\d+)", [this](const auto& req, auto& res) { handle_get_contract(req, res); });
    server.Get("/contracts", [this](const auto& req, auto& res) { handle_get_contracts(req, res); });
    server.Get("/contracts/pending", [this](const auto& req, auto& res) { handle_get_pending_contracts(req, res); });
    server.Post("/contracts/(\\d+)/cancel", [this](const auto& req, auto& res) { handle_cancel_contract(req, res); });
    server.Post("/contracts/(\\d+)/approve", [this](const auto& req, auto& res) { handle_approve_contract(req, res); });
    server.Get("/contracts/(\\d+)/history", [this](const auto& req, auto& res) { handle_get_approval_history(req, res); });
}

int ContractHandler::get_current_user_id(const httplib::Request& req) const {
    auto it = req.headers.find("X-User-Id");
    if (it == req.headers.end()) {
        throw std::runtime_error("X-User-Id header is required");
    }
    
    try {
        return std::stoi(it->second);
    } catch (...) {
        throw std::runtime_error("Invalid X-User-Id");
    }
}

void ContractHandler::handle_error(const std::exception& e, httplib::Response& res, int status_code) const {
    res.status = status_code;
    res.set_content("application/json");
    
    json error;
    error["error_code"] = "INVALID_REQUEST";
    error["message"] = e.what();
    
    res.body = error.dump(2);
}

void ContractHandler::handle_success(const json& data, httplib::Response& res, int status_code) const {
    res.status = status_code;
    res.set_content("application/json");
    res.body = data.dump(2);
}

json ContractHandler::contract_to_json(const domain::Contract& contract) const {
    json j;
    j["id"] = contract.id;
    j["title"] = contract.title;
    j["counterparty"] = contract.counterparty;
    j["amount"] = contract.amount;
    j["currency"] = contract.currency;
    j["creator_id"] = contract.creator_id;
    j["department"] = contract.department;
    j["status"] = domain::Contract::status_to_string(contract.status);
    j["created_at"] = contract.created_at;
    j["updated_at"] = contract.updated_at;
    
    return j;
}

json ContractHandler::contract_with_progress_to_json(const domain::Contract& contract, const domain::ContractApprovalProgress& progress) const {
    json j = contract_to_json(contract);
    
    json progress_json;
    progress_json["total_steps"] = progress.total_steps;
    progress_json["current_step"] = progress.current_step;
    progress_json["current_role"] = progress.current_role;
    if (progress.current_approver_id) {
        progress_json["current_approver_id"] = *progress.current_approver_id;
    }
    if (progress.current_approver_name) {
        progress_json["current_approver_name"] = *progress.current_approver_name;
    }
    
    j["approval_progress"] = progress_json;
    
    return j;
}

void ContractHandler::handle_create_contract(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = get_current_user_id(req);
        
        json body = json::parse(req.body);
        
        std::string title = body["title"];
        std::string counterparty = body["counterparty"];
        long long amount = body["amount"];
        std::string currency = body["currency"];
        std::string department = body["department"];
        
        auto contract = contract_service_->create_contract(user_id, title, counterparty, amount, currency, department);
        
        if (contract) {
            handle_success(contract_to_json(*contract), res, 201);
        } else {
            handle_error(std::runtime_error("Failed to create contract"), res, 500);
        }
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_update_contract(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = get_current_user_id(req);
        
        int contract_id = std::stoi(req.matches[1]);
        json body = json::parse(req.body);
        
        std::string title = body["title"];
        std::string counterparty = body["counterparty"];
        long long amount = body["amount"];
        std::string currency = body["currency"];
        std::string department = body["department"];
        
        auto contract = contract_service_->update_contract(user_id, contract_id, title, counterparty, amount, currency, department);
        
        if (contract) {
            handle_success(contract_to_json(*contract), res);
        } else {
            handle_error(std::runtime_error("Failed to update contract"), res, 500);
        }
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_submit_contract(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = get_current_user_id(req);
        
        int contract_id = std::stoi(req.matches[1]);
        
        auto contract = contract_service_->submit_contract(user_id, contract_id);
        
        if (contract) {
            auto progress = contract_service_->get_contract_approval_progress(contract_id);
            if (progress) {
                handle_success(contract_with_progress_to_json(*contract, *progress), res);
            } else {
                handle_success(contract_to_json(*contract), res);
            }
        } else {
            handle_error(std::runtime_error("Failed to submit contract"), res, 500);
        }
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_get_contract(const httplib::Request& req, httplib::Response& res) {
    try {
        int contract_id = std::stoi(req.matches[1]);
        
        auto contract = contract_service_->get_contract(contract_id);
        
        if (contract) {
            auto progress = contract_service_->get_contract_approval_progress(contract_id);
            if (progress) {
                handle_success(contract_with_progress_to_json(*contract, *progress), res);
            } else {
                handle_success(contract_to_json(*contract), res);
            }
        } else {
            handle_error(std::runtime_error("Contract not found"), res, 404);
        }
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_get_contracts(const httplib::Request& req, httplib::Response& res) {
    try {
        domain::ContractQueryParams params;
        
        // Parse query parameters
        if (req.has_param("status")) {
            params.status = req.get_param("status");
        }
        if (req.has_param("creator")) {
            std::string creator = req.get_param("creator");
            if (creator == "me") {
                try {
                    params.creator_id = get_current_user_id(req);
                } catch (...) {
                    // If X-User-Id is not provided and creator=me is requested, return error
                    handle_error(std::runtime_error("X-User-Id header is required for creator=me"), res, 400);
                    return;
                }
            } else {
                try {
                    params.creator_id = std::stoi(creator);
                } catch (...) {
                    handle_error(std::runtime_error("Invalid creator parameter"), res, 400);
                    return;
                }
            }
        }
        if (req.has_param("min_amount")) {
            try {
                params.min_amount = std::stoll(req.get_param("min_amount"));
            } catch (...) {
                handle_error(std::runtime_error("Invalid min_amount parameter"), res, 400);
                return;
            }
        }
        if (req.has_param("max_amount")) {
            try {
                params.max_amount = std::stoll(req.get_param("max_amount"));
            } catch (...) {
                handle_error(std::runtime_error("Invalid max_amount parameter"), res, 400);
                return;
            }
        }
        
        if (req.has_param("page")) {
            try {
                params.page = std::stoi(req.get_param("page"));
            } catch (...) {
                handle_error(std::runtime_error("Invalid page parameter"), res, 400);
                return;
            }
        }
        if (req.has_param("page_size")) {
            try {
                params.page_size = std::stoi(req.get_param("page_size"));
            } catch (...) {
                handle_error(std::runtime_error("Invalid page_size parameter"), res, 400);
                return;
            }
        }
        
        auto contracts = contract_service_->get_contracts(params);
        
        json result;
        result["contracts"] = json::array();
        
        for (const auto& contract : contracts) {
            result["contracts"].push_back(contract_to_json(contract));
        }
        
        result["total"] = contracts.size();
        result["page"] = params.page;
        result["page_size"] = params.page_size;
        
        handle_success(result, res);
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_get_pending_contracts(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = get_current_user_id(req);
        
        int page = 1;
        int page_size = 10;
        
        if (req.has_param("page")) {
            try {
                page = std::stoi(req.get_param("page"));
            } catch (...) {
                handle_error(std::runtime_error("Invalid page parameter"), res, 400);
                return;
            }
        }
        if (req.has_param("page_size")) {
            try {
                page_size = std::stoi(req.get_param("page_size"));
            } catch (...) {
                handle_error(std::runtime_error("Invalid page_size parameter"), res, 400);
                return;
            }
        }
        
        auto contracts = contract_service_->get_pending_approvals(user_id, page, page_size);
        
        json result;
        result["contracts"] = json::array();
        
        for (const auto& contract : contracts) {
            result["contracts"].push_back(contract_to_json(contract));
        }
        
        result["total"] = contracts.size();
        result["page"] = page;
        result["page_size"] = page_size;
        
        handle_success(result, res);
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_cancel_contract(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = get_current_user_id(req);
        
        int contract_id = std::stoi(req.matches[1]);
        
        auto contract = contract_service_->cancel_contract(user_id, contract_id);
        
        if (contract) {
            handle_success(contract_to_json(*contract), res);
        } else {
            handle_error(std::runtime_error("Failed to cancel contract"), res, 500);
        }
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_approve_contract(const httplib::Request& req, httplib::Response& res) {
    try {
        int user_id = get_current_user_id(req);
        
        int contract_id = std::stoi(req.matches[1]);
        json body = json::parse(req.body);
        
        std::string action = body["action"];
        std::optional<std::string> comment;
        std::optional<int> transfer_to_user_id;
        
        if (body.contains("comment")) {
            comment = body["comment"];
        }
        
        if (body.contains("transfer_to_user_id")) {
            transfer_to_user_id = body["transfer_to_user_id"];
        }
        
        auto contract = contract_service_->approve_contract(user_id, contract_id, action, comment, transfer_to_user_id);
        
        if (contract) {
            auto progress = contract_service_->get_contract_approval_progress(contract_id);
            if (progress) {
                handle_success(contract_with_progress_to_json(*contract, *progress), res);
            } else {
                handle_success(contract_to_json(*contract), res);
            }
        } else {
            handle_error(std::runtime_error("Failed to approve contract"), res, 500);
        }
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

void ContractHandler::handle_get_approval_history(const httplib::Request& req, httplib::Response& res) {
    try {
        int contract_id = std::stoi(req.matches[1]);
        
        auto history = contract_service_->get_approval_history(contract_id);
        
        json result;
        result["history"] = json::array();
        
        for (const auto& log : history) {
            json log_json;
            log_json["id"] = log.id;
            log_json["contract_id"] = log.contract_id;
            if (log.step_id) {
                log_json["step_id"] = *log.step_id;
            }
            log_json["operator_id"] = log.operator_id;
            log_json["action"] = domain::ApprovalLog::action_to_string(log.action);
            if (log.comment) {
                log_json["comment"] = *log.comment;
            }
            log_json["created_at"] = log.created_at;
            
            result["history"].push_back(log_json);
        }
        
        handle_success(result, res);
    } catch (const std::exception& e) {
        handle_error(e, res);
    }
}

} // namespace http
