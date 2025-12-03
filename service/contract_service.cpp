#include "service/contract_service.h"
#include <stdexcept>
#include <fmt/format.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace service {

ContractService::ContractService(std::unique_ptr<storage::StorageInterface> storage) 
    : storage_(std::move(storage)) {}

std::vector<std::string> ContractService::generate_approval_steps(long long amount) const {
    std::vector<std::string> steps;
    
    if (amount < 50000) {
        steps.push_back("manager");
    } else if (amount < 200000) {
        steps.push_back("manager");
        steps.push_back("finance_approver");
    } else {
        steps.push_back("manager");
        steps.push_back("finance_approver");
        steps.push_back("legal_approver");
    }
    
    return steps;
}

std::string ContractService::get_current_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now_time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

std::optional<domain::User> ContractService::get_user(int user_id) const {
    return storage_->get_user_by_id(user_id);
}

void ContractService::check_user_authorization(int user_id) const {
    auto user = get_user(user_id);
    if (!user) {
        throw std::runtime_error(fmt::format("User not found: {}", user_id));
    }
}

std::optional<domain::Contract> ContractService::get_contract_with_check(int contract_id) const {
    auto contract = storage_->get_contract_by_id(contract_id);
    if (!contract) {
        throw std::runtime_error(fmt::format("Contract not found: {}", contract_id));
    }
    return contract;
}

std::optional<domain::Contract> ContractService::create_contract(int user_id, const std::string& title, const std::string& counterparty,
                                                                long long amount, const std::string& currency, const std::string& department) {
    check_user_authorization(user_id);
    
    if (title.empty()) {
        throw std::runtime_error("Title cannot be empty");
    }
    if (counterparty.empty()) {
        throw std::runtime_error("Counterparty cannot be empty");
    }
    if (amount <= 0) {
        throw std::runtime_error("Amount must be positive");
    }
    if (currency.empty()) {
        throw std::runtime_error("Currency cannot be empty");
    }
    if (department.empty()) {
        throw std::runtime_error("Department cannot be empty");
    }
    
    std::string timestamp = get_current_timestamp();
    
    domain::Contract contract;
    contract.id = -1;
    contract.title = title;
    contract.counterparty = counterparty;
    contract.amount = amount;
    contract.currency = currency;
    contract.creator_id = user_id;
    contract.department = department;
    contract.status = domain::ContractStatus::DRAFT;
    contract.created_at = timestamp;
    contract.updated_at = timestamp;
    
    int contract_id = storage_->create_contract(contract);
    
    // Get the created contract
    return storage_->get_contract_by_id(contract_id);
}

std::optional<domain::Contract> ContractService::update_contract(int user_id, int contract_id, const std::string& title,
                                                                const std::string& counterparty, long long amount,
                                                                const std::string& currency, const std::string& department) {
    check_user_authorization(user_id);
    
    auto contract = get_contract_with_check(contract_id);
    if (!contract) {
        return std::nullopt;
    }
    
    // Check if contract is in draft status
    if (contract->status != domain::ContractStatus::DRAFT) {
        throw std::runtime_error("Contract can only be updated in draft status");
    }
    
    // Check if user is the creator
    if (contract->creator_id != user_id) {
        throw std::runtime_error("Only contract creator can update the contract");
    }
    
    if (title.empty()) {
        throw std::runtime_error("Title cannot be empty");
    }
    if (counterparty.empty()) {
        throw std::runtime_error("Counterparty cannot be empty");
    }
    if (amount <= 0) {
        throw std::runtime_error("Amount must be positive");
    }
    if (currency.empty()) {
        throw std::runtime_error("Currency cannot be empty");
    }
    if (department.empty()) {
        throw std::runtime_error("Department cannot be empty");
    }
    
    domain::Contract updated_contract = *contract;
    updated_contract.title = title;
    updated_contract.counterparty = counterparty;
    updated_contract.amount = amount;
    updated_contract.currency = currency;
    updated_contract.department = department;
    updated_contract.updated_at = get_current_timestamp();
    
    if (storage_->update_contract(updated_contract)) {
        return storage_->get_contract_by_id(contract_id);
    }
    
    return std::nullopt;
}

std::optional<domain::Contract> ContractService::submit_contract(int user_id, int contract_id) {
    check_user_authorization(user_id);
    
    auto contract = get_contract_with_check(contract_id);
    if (!contract) {
        return std::nullopt;
    }
    
    // Check if contract is in draft status
    if (contract->status != domain::ContractStatus::DRAFT) {
        throw std::runtime_error("Contract can only be submitted in draft status");
    }
    
    // Check if user is the creator
    if (contract->creator_id != user_id) {
        throw std::runtime_error("Only contract creator can submit the contract");
    }
    
    // Generate approval steps
    std::vector<std::string> step_roles = generate_approval_steps(contract->amount);
    
    // Create approval steps
    for (size_t i = 0; i < step_roles.size(); ++i) {
        domain::ApprovalStep step;
        step.contract_id = contract_id;
        step.step_order = i + 1;
        step.role = step_roles[i];
        step.status = domain::ApprovalStepStatus::PENDING;
        
        // Find approver for this role and department
        if (step_roles[i] == "manager") {
            auto approver = storage_->get_user_by_role_and_department("manager", contract->department);
            if (approver) {
                step.approver_id = approver->id;
            }
        } else {
            auto approver = storage_->get_user_by_role_and_department(step_roles[i], "*");
            if (!approver) {
                approver = storage_->get_user_by_role_and_department(step_roles[i], step_roles[i]);
            }
            if (approver) {
                step.approver_id = approver->id;
            }
        }
        
        storage_->create_approval_step(step);
    }
    
    // Update contract status to approving
    storage_->update_contract_status(contract_id, domain::ContractStatus::APPROVING);
    
    // Create approval log
    domain::ApprovalLog log;
    log.contract_id = contract_id;
    log.operator_id = user_id;
    log.action = domain::ApprovalAction::SUBMIT;
    log.created_at = get_current_timestamp();
    storage_->create_approval_log(log);
    
    return storage_->get_contract_by_id(contract_id);
}

std::optional<domain::Contract> ContractService::get_contract(int contract_id) const {
    return storage_->get_contract_by_id(contract_id);
}

std::vector<domain::Contract> ContractService::get_contracts(const domain::ContractQueryParams& params) const {
    return storage_->get_contracts(params);
}

std::vector<domain::Contract> ContractService::get_pending_approvals(int user_id, int page, int page_size) const {
    check_user_authorization(user_id);
    return storage_->get_pending_approvals_for_user(user_id, page, page_size);
}

std::optional<domain::Contract> ContractService::cancel_contract(int user_id, int contract_id) {
    check_user_authorization(user_id);
    
    auto contract = get_contract_with_check(contract_id);
    if (!contract) {
        return std::nullopt;
    }
    
    // Check if user is the creator
    if (contract->creator_id != user_id) {
        throw std::runtime_error("Only contract creator can cancel the contract");
    }
    
    // Check if contract is in draft or rejected status
    if (contract->status != domain::ContractStatus::DRAFT && contract->status != domain::ContractStatus::REJECTED) {
        throw std::runtime_error("Contract can only be cancelled in draft or rejected status");
    }
    
    // Update contract status to cancelled
    storage_->update_contract_status(contract_id, domain::ContractStatus::CANCELLED);
    
    // Create approval log
    domain::ApprovalLog log;
    log.contract_id = contract_id;
    log.operator_id = user_id;
    log.action = domain::ApprovalAction::CANCEL;
    log.created_at = get_current_timestamp();
    storage_->create_approval_log(log);
    
    return storage_->get_contract_by_id(contract_id);
}

std::optional<domain::Contract> ContractService::approve_contract(int user_id, int contract_id, const std::string& action,
                                                                 const std::optional<std::string>& comment,
                                                                 std::optional<int> transfer_to_user_id) {
    check_user_authorization(user_id);
    
    auto contract = get_contract_with_check(contract_id);
    if (!contract) {
        return std::nullopt;
    }
    
    // Check if contract is in approving status
    if (contract->status != domain::ContractStatus::APPROVING) {
        throw std::runtime_error("Contract is not in approving status");
    }
    
    // Get current approval step
    auto current_step = storage_->get_current_approval_step(contract_id);
    if (!current_step) {
        throw std::runtime_error("No current approval step found");
    }
    
    // Check if user is the approver for this step
    if (current_step->approver_id != user_id) {
        throw std::runtime_error("You are not authorized to approve this step");
    }
    
    std::string timestamp = get_current_timestamp();
    
    if (action == "approve") {
        // Update step status to approved
        storage_->update_approval_step_status(current_step->id, domain::ApprovalStepStatus::APPROVED, 
                                             user_id, comment, timestamp);
        
        // Create approval log
        domain::ApprovalLog log;
        log.contract_id = contract_id;
        log.step_id = current_step->id;
        log.operator_id = user_id;
        log.action = domain::ApprovalAction::APPROVE;
        log.comment = comment;
        log.created_at = timestamp;
        storage_->create_approval_log(log);
        
        // Check if all steps are approved
        auto all_steps = storage_->get_approval_steps_by_contract_id(contract_id);
        bool all_approved = true;
        for (const auto& step : all_steps) {
            if (step.status != domain::ApprovalStepStatus::APPROVED) {
                all_approved = false;
                break;
            }
        }
        
        if (all_approved) {
            // All steps approved - contract approved
            storage_->update_contract_status(contract_id, domain::ContractStatus::APPROVED);
        } else {
            // Check if there's a next pending step
            auto next_step = storage_->get_current_approval_step(contract_id);
            // No action needed - contract remains in approving status
        }
        
    } else if (action == "reject") {
        // Update step status to rejected
        storage_->update_approval_step_status(current_step->id, domain::ApprovalStepStatus::REJECTED, 
                                             user_id, comment, timestamp);
        
        // Create approval log
        domain::ApprovalLog log;
        log.contract_id = contract_id;
        log.step_id = current_step->id;
        log.operator_id = user_id;
        log.action = domain::ApprovalAction::REJECT;
        log.comment = comment;
        log.created_at = timestamp;
        storage_->create_approval_log(log);
        
        // Reject the entire contract
        storage_->update_contract_status(contract_id, domain::ContractStatus::REJECTED);
        
    } else if (action == "transfer") {
        if (!transfer_to_user_id) {
            throw std::runtime_error("Transfer user ID is required for transfer action");
        }
        
        // Check if transfer user exists
        auto transfer_user = storage_->get_user_by_id(*transfer_to_user_id);
        if (!transfer_user) {
            throw std::runtime_error(fmt::format("Transfer user not found: {}", *transfer_to_user_id));
        }
        
        // Update step status to transferred and change approver
        domain::ApprovalStep updated_step = *current_step;
        updated_step.status = domain::ApprovalStepStatus::TRANSFERRED;
        updated_step.acted_at = timestamp;
        updated_step.approver_id = *transfer_to_user_id;
        updated_step.comment = comment;
        storage_->update_approval_step(updated_step);
        
        // Create a new pending step for the transferred user
        domain::ApprovalStep new_step;
        new_step.contract_id = contract_id;
        new_step.step_order = current_step->step_order;
        new_step.role = current_step->role;
        new_step.approver_id = *transfer_to_user_id;
        new_step.status = domain::ApprovalStepStatus::PENDING;
        new_step.comment = comment;
        storage_->create_approval_step(new_step);
        
        // Create approval log
        domain::ApprovalLog log;
        log.contract_id = contract_id;
        log.step_id = current_step->id;
        log.operator_id = user_id;
        log.action = domain::ApprovalAction::TRANSFER;
        log.comment = comment;
        log.created_at = timestamp;
        storage_->create_approval_log(log);
        
    } else {
        throw std::runtime_error(fmt::format("Invalid action: {}", action));
    }
    
    return storage_->get_contract_by_id(contract_id);
}

std::vector<domain::ApprovalLog> ContractService::get_approval_history(int contract_id) const {
    return storage_->get_approval_logs_by_contract_id(contract_id);
}

std::optional<domain::ContractApprovalProgress> ContractService::get_contract_approval_progress(int contract_id) const {
    auto contract = storage_->get_contract_by_id(contract_id);
    if (!contract) {
        return std::nullopt;
    }
    
    auto all_steps = storage_->get_approval_steps_by_contract_id(contract_id);
    if (all_steps.empty()) {
        return std::nullopt;
    }
    
    domain::ContractApprovalProgress progress;
    progress.total_steps = all_steps.size();
    
    // Find current step
    for (const auto& step : all_steps) {
        if (step.status == domain::ApprovalStepStatus::PENDING) {
            progress.current_step = step.step_order;
            progress.current_role = step.role;
            progress.current_approver_id = step.approver_id;
            
            if (step.approver_id) {
                auto approver = storage_->get_user_by_id(*step.approver_id);
                if (approver) {
                    progress.current_approver_name = approver->name;
                }
            }
            
            return progress;
        }
    }
    
    // If no pending steps, find the last completed step
    int last_completed = 0;
    for (const auto& step : all_steps) {
        if (step.status == domain::ApprovalStepStatus::APPROVED || 
            step.status == domain::ApprovalStepStatus::REJECTED) {
            last_completed = step.step_order;
        }
    }
    
    progress.current_step = last_completed;
    progress.current_role = "";
    progress.current_approver_id = std::nullopt;
    progress.current_approver_name = std::nullopt;
    
    return progress;
}

} // namespace service
