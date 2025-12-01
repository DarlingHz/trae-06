#ifndef SLEEPSESSIONDAO_H
#define SLEEPSESSIONDAO_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include "model/SleepSession.h"

namespace dao {

class SleepSessionDao {
public:
    SleepSessionDao(sqlite3* db);
    ~SleepSessionDao();

    // 创建睡眠记录表
    bool createTable() const;
    // 插入新睡眠记录
    bool insertSleepSession(const model::SleepSession& session) const;
    // 根据ID查询睡眠记录
    model::SleepSession findSleepSessionById(int id) const;
    // 根据用户ID和日期范围查询睡眠记录
    std::vector<model::SleepSession> findSleepSessionsByUserIdAndDateRange(
        int user_id, const std::string& start_date, const std::string& end_date,
        int page, int page_size) const;
    // 更新睡眠记录
    bool updateSleepSession(const model::SleepSession& session) const;
    // 删除睡眠记录
    bool deleteSleepSession(int id) const;

private:
    sqlite3* db_;
};

} // namespace dao

#endif // SLEEPSESSIONDAO_H
