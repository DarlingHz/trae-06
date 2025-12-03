#pragma once

#include <string>
#include <ctime>
#include <stdexcept>
#include <map>

class RepairOrder {
public:
    enum class Status {
        PendingReview,
        Accepted,
        InRepair,
        WaitingParts,
        Finished,
        Canceled
    };

    int id = 0;
    int deviceId = 0;
    int userId = 0;
    int serviceCenterId = 0;
    Status status = Status::PendingReview;
    std::string problemDescription;
    time_t expectedFinishDate = 0;
    time_t createdAt = 0;
    time_t updatedAt = 0;

    RepairOrder() = default;

    RepairOrder(int id, int deviceId, int userId, int serviceCenterId,
                Status status, const std::string& problemDescription,
                time_t expectedFinishDate, time_t createdAt, time_t updatedAt)
        : id(id), deviceId(deviceId), userId(userId), serviceCenterId(serviceCenterId),
          status(status), problemDescription(problemDescription),
          expectedFinishDate(expectedFinishDate), createdAt(createdAt), updatedAt(updatedAt) {}

    static Status statusFromString(const std::string& str) {
        static const std::map<std::string, Status> statusMap = {
            {"pending_review", Status::PendingReview},
            {"accepted", Status::Accepted},
            {"in_repair", Status::InRepair},
            {"waiting_parts", Status::WaitingParts},
            {"finished", Status::Finished},
            {"canceled", Status::Canceled}
        };
        auto it = statusMap.find(str);
        if (it == statusMap.end()) {
            throw std::runtime_error("Invalid repair status: " + str);
        }
        return it->second;
    }

    static std::string statusToString(Status status) {
        static const std::map<Status, std::string> statusMap = {
            {Status::PendingReview, "pending_review"},
            {Status::Accepted, "accepted"},
            {Status::InRepair, "in_repair"},
            {Status::WaitingParts, "waiting_parts"},
            {Status::Finished, "finished"},
            {Status::Canceled, "canceled"}
        };
        auto it = statusMap.find(status);
        if (it == statusMap.end()) return "pending_review";
        return it->second;
    }

    bool isValid() const {
        return deviceId > 0 && userId > 0 && serviceCenterId > 0 && !problemDescription.empty();
    }
};
