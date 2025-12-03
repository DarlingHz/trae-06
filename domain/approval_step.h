#pragma once
#include <string>
#include <optional>

namespace domain {

enum class ApprovalStepStatus {
    PENDING,
    APPROVED,
    REJECTED,
    TRANSFERRED
};

struct ApprovalStep {
    int id;
    int contract_id;
    int step_order;
    std::string role;
    std::optional<int> approver_id;
    ApprovalStepStatus status;
    std::optional<std::string> comment;
    std::optional<std::string> acted_at;

    // Default constructor
    ApprovalStep() : id(-1), contract_id(-1), step_order(-1), status(ApprovalStepStatus::PENDING) {}
    
    // Constructor with parameters
    ApprovalStep(int id, int contract_id, int step_order, std::string role,
                 std::optional<int> approver_id, ApprovalStepStatus status,
                 std::optional<std::string> comment, std::optional<std::string> acted_at)
        : id(id), contract_id(contract_id), step_order(step_order), role(std::move(role)),
          approver_id(approver_id), status(status), comment(comment), acted_at(acted_at) {}
    
    // Convert ApprovalStepStatus to string
    static std::string status_to_string(ApprovalStepStatus status);
    
    // Convert string to ApprovalStepStatus
    static ApprovalStepStatus string_to_status(const std::string& str);
};

} // namespace domain
