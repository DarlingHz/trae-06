#include "controller/StatsController.h"
#include <iostream>
#include <ctime>
#include <vector>
#include <algorithm>
#include <numeric>
#include <sstream>
#include "util/Utils.h"

namespace controller {

StatsController::StatsController(dao::SleepSessionDao& sleep_session_dao, 
    dao::UserSettingDao& user_setting_dao) 
    : sleep_session_dao_(sleep_session_dao), user_setting_dao_(user_setting_dao) {
}

StatsController::~StatsController() {
}

json StatsController::handleSummary(const std::string& start_date, const std::string& end_date,
    int user_id) const {
    json response;

    // 验证查询参数是否完整
    if (start_date.empty() || end_date.empty()) {
        response["code"] = 400;
        response["message"] = "Missing required query parameters";
        response["data"] = nullptr;
        return response;
    }

    // 根据用户ID和日期范围查询睡眠记录
    std::vector<model::SleepSession> sessions = 
        sleep_session_dao_.findSleepSessionsByUserIdAndDateRange(user_id, start_date, end_date, 1, 1000);

    // 获取用户的睡眠目标设置
    model::UserSetting user_setting = user_setting_dao_.findUserSettingByUserId(user_id);
    double goal_hours_per_day = user_setting.id != -1 ? user_setting.goal_hours_per_day : 8.0;

    // 计算统计数据
    int total_nights = sessions.size();
    double avg_sleep_hours = total_nights > 0 ? calculateAverageSleepHours(sessions) : 0.0;
    double max_sleep_hours = total_nights > 0 ? calculateMaxSleepHours(sessions) : 0.0;
    double min_sleep_hours = total_nights > 0 ? calculateMinSleepHours(sessions) : 0.0;
    std::string avg_bedtime = total_nights > 0 ? calculateAverageBedtime(sessions) : "";
    std::string avg_waketime = total_nights > 0 ? calculateAverageWaketime(sessions) : "";
    double goal_achieved_ratio = total_nights > 0 ? calculateGoalAchievedRatio(sessions, goal_hours_per_day) : 0.0;

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["total_nights"] = total_nights;
    data["avg_sleep_hours"] = avg_sleep_hours;
    data["max_sleep_hours"] = max_sleep_hours;
    data["min_sleep_hours"] = min_sleep_hours;
    data["avg_bedtime"] = avg_bedtime;
    data["avg_waketime"] = avg_waketime;
    data["goal_hours_per_day"] = goal_hours_per_day;
    data["goal_achieved_ratio"] = goal_achieved_ratio;
    response["data"] = data;

    return response;
}

double StatsController::calculateAverageSleepHours(const std::vector<model::SleepSession>& sessions) const {
    std::vector<double> sleep_hours;
    for (const auto& session : sessions) {
        struct tm start_tm, end_tm;
        if (util::time::parseIsoString(session.start_time, start_tm) &&
            util::time::parseIsoString(session.end_time, end_tm)) {
            double hours = util::time::calculateHoursDiff(start_tm, end_tm);
            sleep_hours.push_back(hours);
        }
    }

    if (sleep_hours.empty()) {
        return 0.0;
    }

    double sum = std::accumulate(sleep_hours.begin(), sleep_hours.end(), 0.0);
    return sum / sleep_hours.size();
}

double StatsController::calculateMaxSleepHours(const std::vector<model::SleepSession>& sessions) const {
    std::vector<double> sleep_hours;
    for (const auto& session : sessions) {
        struct tm start_tm, end_tm;
        if (util::time::parseIsoString(session.start_time, start_tm) &&
            util::time::parseIsoString(session.end_time, end_tm)) {
            double hours = util::time::calculateHoursDiff(start_tm, end_tm);
            sleep_hours.push_back(hours);
        }
    }

    if (sleep_hours.empty()) {
        return 0.0;
    }

    return *std::max_element(sleep_hours.begin(), sleep_hours.end());
}

double StatsController::calculateMinSleepHours(const std::vector<model::SleepSession>& sessions) const {
    std::vector<double> sleep_hours;
    for (const auto& session : sessions) {
        struct tm start_tm, end_tm;
        if (util::time::parseIsoString(session.start_time, start_tm) &&
            util::time::parseIsoString(session.end_time, end_tm)) {
            double hours = util::time::calculateHoursDiff(start_tm, end_tm);
            sleep_hours.push_back(hours);
        }
    }

    if (sleep_hours.empty()) {
        return 0.0;
    }

    return *std::min_element(sleep_hours.begin(), sleep_hours.end());
}

std::string StatsController::calculateAverageBedtime(const std::vector<model::SleepSession>& sessions) const {
    std::vector<int> total_seconds;
    for (const auto& session : sessions) {
        struct tm start_tm;
        if (util::time::parseIsoString(session.start_time, start_tm)) {
            int seconds = start_tm.tm_hour * 3600 + start_tm.tm_min * 60 + start_tm.tm_sec;
            total_seconds.push_back(seconds);
        }
    }

    if (total_seconds.empty()) {
        return "";
    }

    int sum = std::accumulate(total_seconds.begin(), total_seconds.end(), 0);
    int avg_seconds = sum / total_seconds.size();

    int hours = avg_seconds / 3600;
    int minutes = (avg_seconds % 3600) / 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":" 
        << std::setw(2) << std::setfill('0') << minutes;

    return oss.str();
}

std::string StatsController::calculateAverageWaketime(const std::vector<model::SleepSession>& sessions) const {
    std::vector<int> total_seconds;
    for (const auto& session : sessions) {
        struct tm end_tm;
        if (util::time::parseIsoString(session.end_time, end_tm)) {
            int seconds = end_tm.tm_hour * 3600 + end_tm.tm_min * 60 + end_tm.tm_sec;
            total_seconds.push_back(seconds);
        }
    }

    if (total_seconds.empty()) {
        return "";
    }

    int sum = std::accumulate(total_seconds.begin(), total_seconds.end(), 0);
    int avg_seconds = sum / total_seconds.size();

    int hours = avg_seconds / 3600;
    int minutes = (avg_seconds % 3600) / 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":" 
        << std::setw(2) << std::setfill('0') << minutes;

    return oss.str();
}

double StatsController::calculateGoalAchievedRatio(const std::vector<model::SleepSession>& sessions,
    double goal_hours) const {
    if (sessions.empty()) {
        return 0.0;
    }

    int achieved_count = 0;
    for (const auto& session : sessions) {
        struct tm start_tm, end_tm;
        if (util::time::parseIsoString(session.start_time, start_tm) &&
            util::time::parseIsoString(session.end_time, end_tm)) {
            double hours = util::time::calculateHoursDiff(start_tm, end_tm);
            if (hours >= goal_hours) {
                achieved_count++;
            }
        }
    }

    return static_cast<double>(achieved_count) / sessions.size();
}

} // namespace controller
