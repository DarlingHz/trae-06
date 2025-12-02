#ifndef MEAL_PLAN_SERVICE_HPP
#define MEAL_PLAN_SERVICE_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "database.hpp"

using json = nlohmann::json;

class MealPlanService {
public:
    MealPlanService() : db_(Database::getInstance()) {}

    // 创建或更新餐食计划
    json createOrUpdateMealPlan(const json& meal_plan_data, int user_id);

    // 获取特定周的餐食计划
    json getMealPlan(const std::string& week_start_date, int user_id);

    // 删除餐食计划
    json deleteMealPlan(const std::string& week_start_date, int user_id);

private:
    Database& db_;

    // 验证餐食计划是否存在
    bool mealPlanExists(const std::string& week_start_date, int user_id);

    // 验证餐食计划条目
    bool validateMealPlanEntries(const json& entries, int user_id);

    // 转义字符串以防止 SQL 注入
    std::string escapeString(const std::string& str);
};

#endif // MEAL_PLAN_SERVICE_HPP
