#include "domain/approval_log.h"
#include <stdexcept>

namespace domain {

std::string ApprovalLog::action_to_string(ApprovalAction action) {
    switch (action) {
        case ApprovalAction::SUBMIT: return "submit";
        case ApprovalAction::APPROVE: return "approve";
        case ApprovalAction::REJECT: return "reject";
        case ApprovalAction::TRANSFER: return "transfer";
        case ApprovalAction::CANCEL: return "cancel";
        default: throw std::invalid_argument("Invalid approval action");
    }
}

ApprovalAction ApprovalLog::string_to_action(const std::string& str) {
    if (str == "submit") return ApprovalAction::SUBMIT;
    if (str == "approve") return ApprovalAction::APPROVE;
    if (str == "reject") return ApprovalAction::REJECT;
    if (str == "transfer") return ApprovalAction::TRANSFER;
    if (str == "cancel") return ApprovalAction::CANCEL;
    throw std::invalid_argument("Invalid approval action string: " + str);
}

} // namespace domain
