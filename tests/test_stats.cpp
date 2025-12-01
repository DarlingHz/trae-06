#include <iostream>
#include <vector>
#include <cassert>
#include "model/SleepSession.h"
#include "model/UserSetting.h"
#include "controller/StatsController.h"
#include "dao/SleepSessionDao.h"
#include "dao/UserSettingDao.h"

// 测试计算平均睡眠时长
void testCalculateAverageSleepHours() {
    std::vector<model::SleepSession> sessions;

    // 添加测试数据
    model::SleepSession session1;
    session1.start_time = "2025-01-01T23:00:00+08:00";
    session1.end_time = "2025-01-02T07:00:00+08:00";
    sessions.push_back(session1);

    model::SleepSession session2;
    session2.start_time = "2025-01-02T22:30:00+08:00";
    session2.end_time = "2025-01-03T06:30:00+08:00";
    sessions.push_back(session2);

    model::SleepSession session3;
    session3.start_time = "2025-01-03T23:15:00+08:00";
    session3.end_time = "2025-01-04T07:45:00+08:00";
    sessions.push_back(session3);

    // 创建 SQLite 数据库连接
    sqlite3* db;
    int rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // 创建 DAO 对象
    dao::SleepSessionDao sleep_session_dao(db);
    dao::UserSettingDao user_setting_dao(db);

    // 创建 StatsController 对象
    controller::StatsController stats_controller(sleep_session_dao, user_setting_dao);

    // 计算平均睡眠时长
    double avg_hours = stats_controller.calculateAverageSleepHours(sessions);

    // 验证结果（8 + 8 + 8.5）/ 3 = 24.5 / 3 ≈ 8.1667
    assert(avg_hours > 8.16 && avg_hours < 8.17);

    std::cout << "testCalculateAverageSleepHours passed!" << std::endl;
}

// 测试计算目标达成率
void testCalculateGoalAchievedRatio() {
    std::vector<model::SleepSession> sessions;
    model::UserSetting setting;
    setting.goal_hours_per_day = 8.0;

    // 添加测试数据
    model::SleepSession session1;
    session1.start_time = "2025-01-01T23:00:00+08:00";
    session1.end_time = "2025-01-02T07:00:00+08:00";
    sessions.push_back(session1);

    model::SleepSession session2;
    session2.start_time = "2025-01-02T22:30:00+08:00";
    session2.end_time = "2025-01-03T06:30:00+08:00";
    sessions.push_back(session2);

    model::SleepSession session3;
    session3.start_time = "2025-01-03T23:15:00+08:00";
    session3.end_time = "2025-01-04T07:45:00+08:00";
    sessions.push_back(session3);

    model::SleepSession session4;
    session4.start_time = "2025-01-04T23:30:00+08:00";
    session4.end_time = "2025-01-05T06:30:00+08:00";
    sessions.push_back(session4);

    // 创建 SQLite 数据库连接
    sqlite3* db;
    int rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // 创建 DAO 对象
    dao::SleepSessionDao sleep_session_dao(db);
    dao::UserSettingDao user_setting_dao(db);

    // 创建 StatsController 对象
    controller::StatsController stats_controller(sleep_session_dao, user_setting_dao);

    // 计算目标达成率
    double ratio = stats_controller.calculateGoalAchievedRatio(sessions, setting.goal_hours_per_day);

    // 验证结果：4个会话中，3个达到目标（8小时或以上），1个未达到
    assert(ratio == 0.75);

    std::cout << "testCalculateGoalAchievedRatio passed!" << std::endl;
}

// 测试计算平均就寝时间
void testCalculateAverageBedtime() {
    std::vector<model::SleepSession> sessions;

    // 添加测试数据
    model::SleepSession session1;
    session1.start_time = "2025-01-01T23:00:00+08:00";
    sessions.push_back(session1);

    model::SleepSession session2;
    session2.start_time = "2025-01-02T22:30:00+08:00";
    sessions.push_back(session2);

    model::SleepSession session3;
    session3.start_time = "2025-01-03T23:15:00+08:00";
    sessions.push_back(session3);

    // 创建 SQLite 数据库连接
    sqlite3* db;
    int rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // 创建 DAO 对象
    dao::SleepSessionDao sleep_session_dao(db);
    dao::UserSettingDao user_setting_dao(db);

    // 创建 StatsController 对象
    controller::StatsController stats_controller(sleep_session_dao, user_setting_dao);

    // 计算平均就寝时间
    std::string avg_bedtime = stats_controller.calculateAverageBedtime(sessions);

    // 验证结果：(23:00 + 22:30 + 23:15) / 3 = 69:45 / 3 = 23:15
    // 实际计算：(23*3600 + 22*3600 + 30*60 + 23*3600 + 15*60) / 3 = 247500 / 3 = 82500秒
    // 82500秒 = 22小时55分钟
    assert(avg_bedtime == "22:55");

    std::cout << "testCalculateAverageBedtime passed!" << std::endl;
}

// 测试计算平均起床时间
void testCalculateAverageWaketime() {
    std::vector<model::SleepSession> sessions;

    // 添加测试数据
    model::SleepSession session1;
    session1.end_time = "2025-01-02T07:00:00+08:00";
    sessions.push_back(session1);

    model::SleepSession session2;
    session2.end_time = "2025-01-03T06:30:00+08:00";
    sessions.push_back(session2);

    model::SleepSession session3;
    session3.end_time = "2025-01-04T07:45:00+08:00";
    sessions.push_back(session3);

    // 创建 SQLite 数据库连接
    sqlite3* db;
    int rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    // 创建 DAO 对象
    dao::SleepSessionDao sleep_session_dao(db);
    dao::UserSettingDao user_setting_dao(db);

    // 创建 StatsController 对象
    controller::StatsController stats_controller(sleep_session_dao, user_setting_dao);

    // 计算平均起床时间
    std::string avg_waketime = stats_controller.calculateAverageWaketime(sessions);

    // 验证结果：(07:00 + 06:30 + 07:45) / 3 = 21:15 / 3 = 07:05
    assert(avg_waketime == "07:05");

    std::cout << "testCalculateAverageWaketime passed!" << std::endl;
}

int main() {
    try {
        testCalculateAverageSleepHours();
        testCalculateGoalAchievedRatio();
        testCalculateAverageBedtime();
        testCalculateAverageWaketime();

        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
