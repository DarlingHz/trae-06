#pragma once
#include <string>

namespace domain {

enum class ApprovalAction {
    SUBMIT,
    APPROVE,
    REJECT,
    TRANSFER,
    CANCEL
};

struct ApprovalLog {
    int id;
    int contract_id;
    std::optional<int> step_id;
    int operator_id;
    ApprovalAction action;
    std::optional<std::string> comment;
    std::string created_at;

    // Default constructor
    ApprovalLog() : id(-1), contract_id(-1), operator_id(-1), action(ApprovalAction::SUBMIT) {}
    
    // Constructor with parameters
    ApprovalLog(int id, int contract_id, std::optional<int> step_id, int operator_id,
                ApprovalAction action, std::optional<std::string> comment, std::string created_at)
        : id(id), contract_id(contract_id), step_id(step_id), operator_id(operator_id),
          action(action), comment(comment), created_at(std::move(created_at)) {}
    
    // Convert ApprovalAction to string
    static std::string action_to_string(ApprovalAction action);
    
    // Convert string to ApprovalAction
    static ApprovalAction string_to_action(const std::string& str);
};

} // namespace domain
