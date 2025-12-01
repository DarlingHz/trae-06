#include "controller/UserSettingController.h"
#include <iostream>
#include <ctime>
#include "util/Utils.h"

namespace controller {

UserSettingController::UserSettingController(dao::UserSettingDao& user_setting_dao) 
    : user_setting_dao_(user_setting_dao) {
}

UserSettingController::~UserSettingController() {
}

json UserSettingController::handleGet(int user_id) const {
    json response;

    // 根据用户ID查询用户设置
    model::UserSetting user_setting = user_setting_dao_.findUserSettingByUserId(user_id);

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["goal_hours_per_day"] = user_setting.id != -1 ? user_setting.goal_hours_per_day : 8.0;
    response["data"] = data;

    return response;
}

json UserSettingController::handleUpdate(const json& request, int user_id) const {
    json response;

    // 验证请求参数是否完整
    if (!request.contains("goal_hours_per_day") || !request["goal_hours_per_day"].is_number()) {
        response["code"] = 400;
        response["message"] = "Missing or invalid goal_hours_per_day parameter";
        response["data"] = nullptr;
        return response;
    }

    // 获取请求参数
    double goal_hours_per_day = request["goal_hours_per_day"];

    // 验证目标睡眠时长是否合理
    if (goal_hours_per_day < 1.0 || goal_hours_per_day > 24.0) {
        response["code"] = 400;
        response["message"] = "Goal hours per day must be between 1.0 and 24.0";
        response["data"] = nullptr;
        return response;
    }

    // 获取当前时间并格式化为ISO 8601字符串
    std::time_t now = std::time(nullptr);
    struct tm tm = *std::localtime(&now);
    std::string updated_at = util::time::toIsoString(tm);

    // 创建用户设置对象
    model::UserSetting user_setting;
    user_setting.user_id = user_id;
    user_setting.goal_hours_per_day = goal_hours_per_day;
    user_setting.updated_at = updated_at;

    // 保存用户设置到数据库
    bool upsert_success = user_setting_dao_.upsertUserSetting(user_setting);
    if (!upsert_success) {
        response["code"] = 500;
        response["message"] = "Failed to update user setting";
        response["data"] = nullptr;
        return response;
    }

    // 生成成功响应
    response["code"] = 0;
    response["message"] = "ok";
    json data;
    data["message"] = "User setting updated successfully";
    response["data"] = data;

    return response;
}

} // namespace controller
