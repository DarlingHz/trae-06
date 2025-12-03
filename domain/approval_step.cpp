#include "domain/approval_step.h"
#include <stdexcept>

namespace domain {

std::string ApprovalStep::status_to_string(ApprovalStepStatus status) {
    switch (status) {
        case ApprovalStepStatus::PENDING: return "pending";
        case ApprovalStepStatus::APPROVED: return "approved";
        case ApprovalStepStatus::REJECTED: return "rejected";
        case ApprovalStepStatus::TRANSFERRED: return "transferred";
        default: throw std::invalid_argument("Invalid approval step status");
    }
}

ApprovalStepStatus ApprovalStep::string_to_status(const std::string& str) {
    if (str == "pending") return ApprovalStepStatus::PENDING;
    if (str == "approved") return ApprovalStepStatus::APPROVED;
    if (str == "rejected") return ApprovalStepStatus::REJECTED;
    if (str == "transferred") return ApprovalStepStatus::TRANSFERRED;
    throw std::invalid_argument("Invalid approval step status string: " + str);
}

} // namespace domain
