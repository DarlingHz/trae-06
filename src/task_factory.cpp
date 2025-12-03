#include "job_service/task_factory.h"
#include <stdexcept>

namespace job_service {

void TaskFactory::register_task_type(const JobType& type, TaskCreator creator) {
    std::lock_guard<std::mutex> lock(mutex_);
    creators_[type] = std::move(creator);
}

TaskPtr TaskFactory::create_task(const JobType& type) const {
    auto it = creators_.find(type);
    if (it == creators_.end()) {
        throw TaskNotFoundException(type);
    }
    return it->second();
}

bool TaskFactory::has_task_type(const JobType& type) const {
    return creators_.find(type) != creators_.end();
}

std::vector<JobType> TaskFactory::get_supported_types() const {
    std::vector<JobType> types;
    types.reserve(creators_.size());
    
    for (const auto& pair : creators_) {
        types.push_back(pair.first);
    }
    
    return types;
}

TaskNotFoundException::TaskNotFoundException(const JobType& type)
    : std::runtime_error(std::string("Unknown task type: ") + type) {
}

InvalidTaskParametersException::InvalidTaskParametersException(const std::string& message)
    : std::runtime_error(std::string("Invalid task parameters: ") + message) {
}

} // namespace job_service
