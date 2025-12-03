#pragma once

#include <string>
#include <ctime>

class RepairStatusHistory {
public:
    int id = 0;
    int repairOrderId = 0;
    std::string status;
    std::string note;
    time_t createdAt = 0;
    std::string operatorName;

    RepairStatusHistory() = default;

    RepairStatusHistory(int id, int repairOrderId, const std::string& status,
                       const std::string& note, time_t createdAt, const std::string& operatorName)
        : id(id), repairOrderId(repairOrderId), status(status), note(note),
          createdAt(createdAt), operatorName(operatorName) {}

    bool isValid() const {
        return repairOrderId > 0 && !status.empty();
    }
};
