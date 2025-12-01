#include "controller/SleepSessionController.h"
#include <iostream>
#include <ctime>
#include "util/Utils.h"

namespace controller {

SleepSessionController::SleepSessionController(dao::SleepSessionDao& sleep_session_dao) 
    : sleep_session_dao_(sleep_session_dao) {
}

SleepSessionController::~SleepSessionController() {
}

json SleepSessionController::handleCreate(const json& request, int user_id) const {
    json response;

    // 验证请求参数是否完整
    if (!request.contains("start_time") || !request.contains("end_time") ||
        !request.contains("quality")) {
        response["code"] = 400;
        response["message"] = "Missing required parameters";
        response["data"] = nullptr;
        return response;
    }

    // 获取请求参数
    std::string start_time = request["start_time"];
    std::string end_time = request["end_time"];
    int quality = request["quality"];
    std::vector<std::string> tags;
    if (request.contains("tags") && request["tags"].is_array()) {
        tags = request["tags"].get<std::vector<std::string>>();
    }
    std::string note = "";
    if (request.contains("note") && request["note"].is_string()) {
        note = request["note"];
    }

    // 验证质量评分是否在0-10之间
    if (quality < 0 || quality > 10) {
        response["code"] = 400;
        response["message"] = "Quality score must be between 0 and 10";
        response["data"] = nullptr;
        return response;
    }

    // 创建睡眠记录对象
    model::SleepSession session;
    session.user_id = user_id;
    session.start_time = start_time;
    session.end_time = end_time;
    session.quality = quality;
    session.tags = tags;
    session.note = note;

    // 验证睡眠记录的时间格式和逻辑
    if (!validateSleepSession(session)) {
        response["code"] = 400;
        response["message"] = "Invalid sleep session time format or logic";
        response["data"] = nullptr;
        return response;
    }

    // 保存睡眠记录到数据库
    bool insert_success = sleep_session_dao_.insertSleepSession(session);
    if (!insert_success) {
        response["code"] = 500;
        response["message"] = "Failed to create sleep session";
        response["data"] = nullptr;
        return response;
    }

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["message"] = "Sleep session created successfully";
    response["data"] = data;

    return response;
}

json SleepSessionController::handleQuery(const std::string& start_date, const std::string& end_date,
    int page, int page_size, int user_id) const {
    json response;

    // 验证查询参数是否完整
    if (start_date.empty() || end_date.empty()) {
        response["code"] = 400;
        response["message"] = "Missing required query parameters";
        response["data"] = nullptr;
        return response;
    }

    // 验证分页参数是否合理
    if (page < 1 || page_size < 1 || page_size > 100) {
        response["code"] = 400;
        response["message"] = "Invalid page or page_size parameter";
        response["data"] = nullptr;
        return response;
    }

    // 根据用户ID和日期范围查询睡眠记录
    std::vector<model::SleepSession> sessions = 
        sleep_session_dao_.findSleepSessionsByUserIdAndDateRange(user_id, start_date, end_date, page, page_size);

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["sessions"] = sessions;
    data["page"] = page;
    data["page_size"] = page_size;
    data["total"] = sessions.size(); // 实际应用中应该查询总记录数
    response["data"] = data;

    return response;
}

json SleepSessionController::handleUpdate(int id, const json& request, int user_id) const {
    json response;

    // 根据ID查询睡眠记录
    model::SleepSession existing_session = sleep_session_dao_.findSleepSessionById(id);
    if (existing_session.id == -1) {
        response["code"] = 404;
        response["message"] = "Sleep session not found";
        response["data"] = nullptr;
        return response;
    }

    // 验证睡眠记录是否属于当前用户
    if (existing_session.user_id != user_id) {
        response["code"] = 401;
        response["message"] = "You do not have permission to update this sleep session";
        response["data"] = nullptr;
        return response;
    }

    // 更新睡眠记录的字段
    if (request.contains("start_time") && request["start_time"].is_string()) {
        existing_session.start_time = request["start_time"];
    }
    if (request.contains("end_time") && request["end_time"].is_string()) {
        existing_session.end_time = request["end_time"];
    }
    if (request.contains("quality") && request["quality"].is_number_integer()) {
        existing_session.quality = request["quality"];
    }
    if (request.contains("tags") && request["tags"].is_array()) {
        existing_session.tags = request["tags"].get<std::vector<std::string>>();
    }
    if (request.contains("note") && request["note"].is_string()) {
        existing_session.note = request["note"];
    }

    // 验证质量评分是否在0-10之间
    if (existing_session.quality < 0 || existing_session.quality > 10) {
        response["code"] = 400;
        response["message"] = "Quality score must be between 0 and 10";
        response["data"] = nullptr;
        return response;
    }

    // 验证睡眠记录的时间格式和逻辑
    if (!validateSleepSession(existing_session)) {
        response["code"] = 400;
        response["message"] = "Invalid sleep session time format or logic";
        response["data"] = nullptr;
        return response;
    }

    // 更新睡眠记录到数据库
    bool update_success = sleep_session_dao_.updateSleepSession(existing_session);
    if (!update_success) {
        response["code"] = 500;
        response["message"] = "Failed to update sleep session";
        response["data"] = nullptr;
        return response;
    }

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["message"] = "Sleep session updated successfully";
    response["data"] = data;

    return response;
}

json SleepSessionController::handleDelete(int id, int user_id) const {
    json response;

    // 根据ID查询睡眠记录
    model::SleepSession existing_session = sleep_session_dao_.findSleepSessionById(id);
    if (existing_session.id == -1) {
        response["code"] = 404;
        response["message"] = "Sleep session not found";
        response["data"] = nullptr;
        return response;
    }

    // 验证睡眠记录是否属于当前用户
    if (existing_session.user_id != user_id) {
        response["code"] = 401;
        response["message"] = "You do not have permission to delete this sleep session";
        response["data"] = nullptr;
        return response;
    }

    // 删除睡眠记录
    bool delete_success = sleep_session_dao_.deleteSleepSession(id);
    if (!delete_success) {
        response["code"] = 500;
        response["message"] = "Failed to delete sleep session";
        response["data"] = nullptr;
        return response;
    }

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["message"] = "Sleep session deleted successfully";
    response["data"] = data;

    return response;
}

bool SleepSessionController::validateSleepSession(const model::SleepSession& session) const {
    // 验证开始时间和结束时间的格式是否正确
    struct tm start_tm, end_tm;
    if (!util::time::parseIsoString(session.start_time, start_tm) ||
        !util::time::parseIsoString(session.end_time, end_tm)) {
        return false;
    }

    // 验证结束时间是否晚于开始时间
    time_t start_time = mktime(const_cast<struct tm*>(&start_tm));
    time_t end_time = mktime(const_cast<struct tm*>(&end_tm));
    if (end_time <= start_time) {
        return false;
    }

    return true;
}

} // namespace controller
