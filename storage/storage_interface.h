#pragma once
#include <vector>
#include <optional>
#include "domain/user.h"
#include "domain/contract.h"
#include "domain/approval_step.h"
#include "domain/approval_log.h"

namespace storage {

class StorageInterface {
public:
    virtual ~StorageInterface() = default;
    
    // User operations
    virtual bool initialize() = 0;
    virtual std::optional<domain::User> get_user_by_id(int user_id) const = 0;
    virtual std::optional<domain::User> get_user_by_role_and_department(const std::string& role, const std::string& department) const = 0;
    virtual int get_total_users() const = 0;
    virtual void insert_default_users() = 0;
    
    // Contract operations
    virtual int create_contract(const domain::Contract& contract) = 0;
    virtual std::optional<domain::Contract> get_contract_by_id(int contract_id) const = 0;
    virtual std::vector<domain::Contract> get_contracts(const domain::ContractQueryParams& params) const = 0;
    virtual bool update_contract(const domain::Contract& contract) = 0;
    virtual bool update_contract_status(int contract_id, domain::ContractStatus status) = 0;
    
    // Approval step operations
    virtual std::vector<domain::ApprovalStep> get_approval_steps_by_contract_id(int contract_id) const = 0;
    virtual std::optional<domain::ApprovalStep> get_current_approval_step(int contract_id) const = 0;
    virtual int create_approval_step(const domain::ApprovalStep& step) = 0;
    virtual bool update_approval_step(const domain::ApprovalStep& step) = 0;
    virtual bool update_approval_step_status(int step_id, domain::ApprovalStepStatus status, int approver_id, const std::optional<std::string>& comment, const std::string& acted_at) = 0;
    
    // Approval log operations
    virtual int create_approval_log(const domain::ApprovalLog& log) = 0;
    virtual std::vector<domain::ApprovalLog> get_approval_logs_by_contract_id(int contract_id) const = 0;
    
    // Pending approvals for user
    virtual std::vector<domain::Contract> get_pending_approvals_for_user(int user_id, int page, int page_size) const = 0;
};

} // namespace storage
