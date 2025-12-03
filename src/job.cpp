#include "job_service/job.h"
#include "job_service/utils.h"
#include <stdexcept>
#include <iomanip>
#include <sstream>

namespace job_service {

using namespace std::chrono;

std::string job_status_to_string(JobStatus status) {
    switch (status) {
        case JobStatus::QUEUED: return "queued";
        case JobStatus::RUNNING: return "running";
        case JobStatus::DONE: return "done";
        case JobStatus::FAILED: return "failed";
        case JobStatus::CANCELED: return "canceled";
        default: return "unknown";
    }
}

std::optional<JobStatus> string_to_job_status(const std::string& str) {
    std::string lower_str = utils::to_lowercase(str);
    if (lower_str == "queued") return JobStatus::QUEUED;
    if (lower_str == "running") return JobStatus::RUNNING;
    if (lower_str == "done") return JobStatus::DONE;
    if (lower_str == "failed") return JobStatus::FAILED;
    if (lower_str == "canceled") return JobStatus::CANCELED;
    return std::nullopt;
}

Job::Job(const std::string& job_id, const JobType& type, const nlohmann::json& payload, JobPriority priority)
    : job_id_(job_id),
      type_(type),
      payload_(payload),
      priority_(priority),
      status_(JobStatus::QUEUED),
      created_at_(system_clock::now()),
      started_at_(std::nullopt),
      finished_at_(std::nullopt),
      result_(std::nullopt),
      error_(std::nullopt),
      cancel_requested_(false) {
}

const std::string& Job::get_job_id() const {
    return job_id_;
}

const JobType& Job::get_type() const {
    return type_;
}

const nlohmann::json& Job::get_payload() const {
    return payload_;
}

JobPriority Job::get_priority() const {
    return priority_;
}

JobStatus Job::get_status() const {
    return status_;
}

void Job::set_status(JobStatus status) {
    status_ = status;
}

const system_clock::time_point& Job::get_created_at() const {
    return created_at_;
}

const std::optional<system_clock::time_point>& Job::get_started_at() const {
    return started_at_;
}

void Job::set_started_at(const system_clock::time_point& time) {
    started_at_ = time;
}

const std::optional<system_clock::time_point>& Job::get_finished_at() const {
    return finished_at_;
}

void Job::set_finished_at(const system_clock::time_point& time) {
    finished_at_ = time;
}

const std::optional<nlohmann::json>& Job::get_result() const {
    return result_;
}

void Job::set_result(const nlohmann::json& result) {
    result_ = result;
}

const std::optional<std::string>& Job::get_error() const {
    return error_;
}

void Job::set_error(const std::string& error) {
    error_ = error;
}

void Job::request_cancel() {
    cancel_requested_ = true;
}

bool Job::is_cancel_requested() const {
    return cancel_requested_;
}

nlohmann::json Job::to_json() const {
    nlohmann::json j;
    j["job_id"] = job_id_;
    j["type"] = type_;
    j["status"] = job_status_to_string(status_);
    j["priority"] = priority_;
    j["created_at"] = utils::time_to_iso_string(created_at_);
    if (started_at_) {
        j["started_at"] = utils::time_to_iso_string(*started_at_);
    }
    if (finished_at_) {
        j["finished_at"] = utils::time_to_iso_string(*finished_at_);
    }
    if (result_) {
        j["result"] = *result_;
    }
    if (error_) {
        j["error"] = *error_;
    }
    j["cancel_requested"] = cancel_requested_;
    return j;
}

Job Job::from_json(const nlohmann::json& json) {
    std::string job_id = json["job_id"].get<std::string>();
    std::string type = json["type"].get<std::string>();
    nlohmann::json payload = json["payload"].get<nlohmann::json>();
    JobPriority priority = json["priority"].get<JobPriority>();
    
    Job job(job_id, type, payload, priority);
    
    if (json.contains("status")) {
        auto status_opt = string_to_job_status(json["status"].get<std::string>());
        if (status_opt) {
            job.status_ = *status_opt;
        }
    }
    
    if (json.contains("created_at")) {
        auto time_opt = utils::iso_string_to_time(json["created_at"].get<std::string>());
        if (time_opt) {
            job.created_at_ = *time_opt;
        }
    }
    
    if (json.contains("started_at")) {
        auto time_opt = utils::iso_string_to_time(json["started_at"].get<std::string>());
        if (time_opt) {
            job.started_at_ = *time_opt;
        }
    }
    
    if (json.contains("finished_at")) {
        auto time_opt = utils::iso_string_to_time(json["finished_at"].get<std::string>());
        if (time_opt) {
            job.finished_at_ = *time_opt;
        }
    }
    
    if (json.contains("result")) {
        job.result_ = json["result"].get<nlohmann::json>();
    }
    
    if (json.contains("error")) {
        job.error_ = json["error"].get<std::string>();
    }
    
    if (json.contains("cancel_requested")) {
        job.cancel_requested_ = json["cancel_requested"].get<bool>();
    }
    
    return job;
}

} // namespace job_service
