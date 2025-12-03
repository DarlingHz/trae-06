#pragma once

#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>
#include "service/contract_service.h"

namespace http {

using json = nlohmann::json;

class ContractHandler {
public:
    ContractHandler(std::unique_ptr<service::ContractService> contract_service) 
        : contract_service_(std::move(contract_service)) {}
    
    void init_routes(httplib::Server& server);
    
private:
    std::unique_ptr<service::ContractService> contract_service_;
    
    // Helper methods
    int get_current_user_id(const httplib::Request& req) const;
    void handle_error(const std::exception& e, httplib::Response& res, int status_code = 400) const;
    void handle_success(const json& data, httplib::Response& res, int status_code = 200) const;
    json contract_to_json(const domain::Contract& contract) const;
    json contract_with_progress_to_json(const domain::Contract& contract, const domain::ContractApprovalProgress& progress) const;
    
    // Route handlers
    void handle_create_contract(const httplib::Request& req, httplib::Response& res);
    void handle_update_contract(const httplib::Request& req, httplib::Response& res);
    void handle_submit_contract(const httplib::Request& req, httplib::Response& res);
    void handle_get_contract(const httplib::Request& req, httplib::Response& res);
    void handle_get_contracts(const httplib::Request& req, httplib::Response& res);
    void handle_get_pending_contracts(const httplib::Request& req, httplib::Response& res);
    void handle_cancel_contract(const httplib::Request& req, httplib::Response& res);
    void handle_approve_contract(const httplib::Request& req, httplib::Response& res);
    void handle_get_approval_history(const httplib::Request& req, httplib::Response& res);
};

} // namespace http
