#ifndef SLEEPSESSIONCONTROLLER_H
#define SLEEPSESSIONCONTROLLER_H

#include <string>
#include <nlohmann/json.hpp>
#include "dao/SleepSessionDao.h"

using json = nlohmann::json;

namespace controller {

class SleepSessionController {
public:
    SleepSessionController(dao::SleepSessionDao& sleep_session_dao);
    ~SleepSessionController();

    // 处理新增睡眠记录请求
    json handleCreate(const json& request, int user_id) const;
    // 处理查询睡眠记录请求
    json handleQuery(const std::string& start_date, const std::string& end_date,
        int page, int page_size, int user_id) const;
    // 处理更新睡眠记录请求
    json handleUpdate(int id, const json& request, int user_id) const;
    // 处理删除睡眠记录请求
    json handleDelete(int id, int user_id) const;

private:
    dao::SleepSessionDao& sleep_session_dao_;

    // 验证睡眠记录的时间格式和逻辑
    bool validateSleepSession(const model::SleepSession& session) const;
};

} // namespace controller

#endif // SLEEPSESSIONCONTROLLER_H
