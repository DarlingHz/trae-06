#include "job.h"
#include <sstream>

namespace recruitment {

std::string Job::toJson() const {
    std::stringstream ss;
    ss << ",";
    ss << "\"id\": " << id_ << ",";
    ss << "\"company_id\": " << company_id_ << ",";
    ss << "\"title\": \"" << title_ << ",";
    ss << "\"location\": \"" << location_ << ",";
    ss << "\"salary_range\": \"" << salary_range_ << ",";
    ss << "\"description\": \"" << description_ << ",";
    ss << "\"required_skills\": \"" << required_skills_ << ",";
    ss << "\"is_open\": " << (is_open_ ? "true" : "false") << ",";
    ss << "\"created_at\": \"" << created_at_ << ",";
    ss << "\"updated_at\": \"" << updated_at_ << "\"";
    ss << "}";
    return ss.str();
}

} // namespace recruitment