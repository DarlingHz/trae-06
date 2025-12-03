#include "domain/contract.h"
#include <stdexcept>

namespace domain {

std::string Contract::status_to_string(ContractStatus status) {
    switch (status) {
        case ContractStatus::DRAFT: return "draft";
        case ContractStatus::SUBMITTED: return "submitted";
        case ContractStatus::APPROVING: return "approving";
        case ContractStatus::APPROVED: return "approved";
        case ContractStatus::REJECTED: return "rejected";
        case ContractStatus::CANCELLED: return "cancelled";
        default: throw std::invalid_argument("Invalid contract status");
    }
}

ContractStatus Contract::string_to_status(const std::string& str) {
    if (str == "draft") return ContractStatus::DRAFT;
    if (str == "submitted") return ContractStatus::SUBMITTED;
    if (str == "approving") return ContractStatus::APPROVING;
    if (str == "approved") return ContractStatus::APPROVED;
    if (str == "rejected") return ContractStatus::REJECTED;
    if (str == "cancelled") return ContractStatus::CANCELLED;
    throw std::invalid_argument("Invalid contract status string: " + str);
}

} // namespace domain
