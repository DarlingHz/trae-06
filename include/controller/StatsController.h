#ifndef STATSCONTROLLER_H
#define STATSCONTROLLER_H

#include <string>
#include <nlohmann/json.hpp>
#include "dao/SleepSessionDao.h"
#include "dao/UserSettingDao.h"

using json = nlohmann::json;

namespace controller {

class StatsController {
public:
    StatsController(dao::SleepSessionDao& sleep_session_dao, 
        dao::UserSettingDao& user_setting_dao);
    ~StatsController();

    // 处理睡眠统计请求
    json handleSummary(const std::string& start_date, const std::string& end_date,
        int user_id) const;

public:
    // 计算平均睡眠时长
    double calculateAverageSleepHours(const std::vector<model::SleepSession>& sessions) const;
    // 计算最长睡眠时长
    double calculateMaxSleepHours(const std::vector<model::SleepSession>& sessions) const;
    // 计算最短睡眠时长
    double calculateMinSleepHours(const std::vector<model::SleepSession>& sessions) const;
    // 计算平均入睡时间
    std::string calculateAverageBedtime(const std::vector<model::SleepSession>& sessions) const;
    // 计算平均起床时间
    std::string calculateAverageWaketime(const std::vector<model::SleepSession>& sessions) const;
    // 计算目标达成率
    double calculateGoalAchievedRatio(const std::vector<model::SleepSession>& sessions,
        double goal_hours) const;

private:
    dao::SleepSessionDao& sleep_session_dao_;
    dao::UserSettingDao& user_setting_dao_;
};

} // namespace controller

#endif // STATSCONTROLLER_H
