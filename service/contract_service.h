#pragma once
#include <vector>
#include <optional>
#include <string>
#include "storage/storage_interface.h"
#include "domain/user.h"
#include "domain/contract.h"
#include "domain/approval_step.h"
#include "domain/approval_log.h"

namespace service {

class ContractService {
private:
    std::unique_ptr<storage::StorageInterface> storage_;
    
    // Generate approval steps based on contract amount
    std::vector<std::string> generate_approval_steps(long long amount) const;
    
    // Get current ISO 8601 timestamp
    std::string get_current_timestamp() const;
    
    // Get user info
    std::optional<domain::User> get_user(int user_id) const;
    
    // Check if user is authorized to perform action
    void check_user_authorization(int user_id) const;
    
    // Get contract with check
    std::optional<domain::Contract> get_contract_with_check(int contract_id) const;
    
public:
    explicit ContractService(std::unique_ptr<storage::StorageInterface> storage);
    ~ContractService() = default;
    
    // Prevent copying
    ContractService(const ContractService&) = delete;
    ContractService& operator=(const ContractService&) = delete;
    
    // Allow moving
    ContractService(ContractService&&) noexcept = default;
    ContractService& operator=(ContractService&&) noexcept = default;
    
    // Contract operations
    std::optional<domain::Contract> create_contract(int user_id, const std::string& title, const std::string& counterparty, 
                                                  long long amount, const std::string& currency, const std::string& department);
    
    std::optional<domain::Contract> update_contract(int user_id, int contract_id, const std::string& title, 
                                                  const std::string& counterparty, long long amount, 
                                                  const std::string& currency, const std::string& department);
    
    std::optional<domain::Contract> submit_contract(int user_id, int contract_id);
    
    std::optional<domain::Contract> get_contract(int contract_id) const;
    
    std::vector<domain::Contract> get_contracts(const domain::ContractQueryParams& params) const;
    
    std::vector<domain::Contract> get_pending_approvals(int user_id, int page, int page_size) const;
    
    std::optional<domain::Contract> cancel_contract(int user_id, int contract_id);
    
    // Approval operations
    std::optional<domain::Contract> approve_contract(int user_id, int contract_id, const std::string& action, 
                                                   const std::optional<std::string>& comment, 
                                                   std::optional<int> transfer_to_user_id = std::nullopt);
    
    std::vector<domain::ApprovalLog> get_approval_history(int contract_id) const;
    
    std::optional<domain::ContractApprovalProgress> get_contract_approval_progress(int contract_id) const;
};

} // namespace service
