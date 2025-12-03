#pragma once
#include <string>
#include <optional>
#include <vector>

namespace domain {

enum class ContractStatus {
    DRAFT,
    SUBMITTED,
    APPROVING,
    APPROVED,
    REJECTED,
    CANCELLED
};

struct Contract {
    int id;
    std::string title;
    std::string counterparty;
    long long amount;
    std::string currency;
    int creator_id;
    std::string department;
    ContractStatus status;
    std::string created_at;
    std::string updated_at;

    // Default constructor
    Contract() : id(-1), amount(0), creator_id(-1) {}
    
    // Constructor with parameters
    Contract(int id, std::string title, std::string counterparty, long long amount, std::string currency,
             int creator_id, std::string department, ContractStatus status, std::string created_at, std::string updated_at)
        : id(id), title(std::move(title)), counterparty(std::move(counterparty)), amount(amount),
          currency(std::move(currency)), creator_id(creator_id), department(std::move(department)),
          status(status), created_at(std::move(created_at)), updated_at(std::move(updated_at)) {}
    
    // Convert ContractStatus to string
    static std::string status_to_string(ContractStatus status);
    
    // Convert string to ContractStatus
    static ContractStatus string_to_status(const std::string& str);
};

// Struct for contract list query parameters
struct ContractQueryParams {
    std::optional<std::string> status;
    std::optional<int> creator_id;
    std::optional<long long> min_amount;
    std::optional<long long> max_amount;
    int page = 1;
    int page_size = 10;
};

// Struct for contract approval progress
struct ContractApprovalProgress {
    int current_step;
    int total_steps;
    std::string current_role;
    std::optional<int> current_approver_id;
    std::optional<std::string> current_approver_name;
};

} // namespace domain
