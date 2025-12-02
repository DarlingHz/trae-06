#include "meal_plan_service.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <regex>

using namespace std;

json MealPlanService::createOrUpdateMealPlan(const json& meal_plan_data, int user_id) {
    json response;

    try {
        // 验证必填字段
        if (!meal_plan_data.contains("weekStartDate") || !meal_plan_data.contains("entries")) {
            response["errorCode"] = "MISSING_FIELDS";
            response["message"] = "缺少必填字段: weekStartDate, entries";
            return response;
        }

        string week_start_date = meal_plan_data["weekStartDate"];
        json entries = meal_plan_data["entries"];

        // 验证字段长度
        if (week_start_date.empty() || entries.empty()) {
            response["errorCode"] = "EMPTY_FIELDS";
            response["message"] = "字段不能为空";
            return response;
        }

        // 验证 weekStartDate 格式（YYYY-MM-DD）
        regex date_regex("\\d{4}-\\d{2}-\\d{2}");
        if (!regex_match(week_start_date, date_regex)) {
            response["errorCode"] = "INVALID_DATE_FORMAT";
            response["message"] = "日期格式不正确，需要 YYYY-MM-DD 格式";
            return response;
        }

        // 验证餐食计划条目
        if (!validateMealPlanEntries(entries, user_id)) {
            response["errorCode"] = "INVALID_ENTRIES";
            response["message"] = "餐食计划条目格式不正确或包含不属于当前用户的菜谱";
            return response;
        }

        // 开始事务
        if (!db_.beginTransaction()) {
            response["errorCode"] = "TRANSACTION_FAILED";
            response["message"] = "事务开始失败";
            return response;
        }

        if (mealPlanExists(week_start_date, user_id)) {
            // 更新现有餐食计划
            stringstream update_sql;
            update_sql << "UPDATE meal_plans SET entries = '" << escapeString(entries.dump()) << "', updated_at = CURRENT_TIMESTAMP WHERE user_id = " << user_id << " AND week_start_date = '" << week_start_date << "';";

            if (!db_.execute(update_sql.str())) {
                db_.rollbackTransaction();
                response["errorCode"] = "UPDATE_FAILED";
                response["message"] = "餐食计划更新失败";
                return response;
            }
        } else {
            // 创建新餐食计划
            stringstream insert_sql;
            insert_sql << "INSERT INTO meal_plans (user_id, week_start_date, entries) VALUES (" << user_id << ", '" << week_start_date << "', '" << escapeString(entries.dump()) << "');";

            if (!db_.execute(insert_sql.str())) {
                db_.rollbackTransaction();
                response["errorCode"] = "INSERT_FAILED";
                response["message"] = "餐食计划创建失败";
                return response;
            }
        }

        // 提交事务
        if (!db_.commitTransaction()) {
            db_.rollbackTransaction();
            response["errorCode"] = "TRANSACTION_COMMIT_FAILED";
            response["message"] = "事务提交失败";
            return response;
        }

        // 返回成功响应
        response["success"] = true;

    } catch (const exception& e) {
        db_.rollbackTransaction();
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "创建或更新餐食计划错误: " << e.what() << endl;
    }

    return response;
}

json MealPlanService::getMealPlan(const string& week_start_date, int user_id) {
    json response;

    try {
        // 验证 weekStartDate 格式（YYYY-MM-DD）
        regex date_regex("\\d{4}-\\d{2}-\\d{2}");
        if (!regex_match(week_start_date, date_regex)) {
            response["errorCode"] = "INVALID_DATE_FORMAT";
            response["message"] = "日期格式不正确，需要 YYYY-MM-DD 格式";
            return response;
        }

        // 查询餐食计划
        json meal_plan;
        stringstream select_sql;
        select_sql << "SELECT id, week_start_date, entries, created_at, updated_at FROM meal_plans WHERE user_id = " << user_id << " AND week_start_date = '" << week_start_date << "';";

        if (!db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 5 && argv[0] != nullptr) {
                meal_plan["id"] = stoi(argv[0]);
                meal_plan["week_start_date"] = argv[1] != nullptr ? argv[1] : "";
                meal_plan["entries"] = argv[2] != nullptr ? json::parse(argv[2]) : json::array();
                meal_plan["created_at"] = argv[3] != nullptr ? argv[3] : "";
                meal_plan["updated_at"] = argv[4] != nullptr ? argv[4] : "";
            }
            return true;
        })) {
            response["errorCode"] = "QUERY_FAILED";
            response["message"] = "查询餐食计划失败";
            return response;
        }

        // 如果找到餐食计划，获取每个 entry 对应的简要菜谱信息
        if (!meal_plan.empty()) {
            json entries = meal_plan["entries"];
            for (auto& entry : entries) {
                if (entry.contains("recipeId")) {
                    int recipe_id = entry["recipeId"];
                    json recipe_info;

                    stringstream recipe_sql;
                    recipe_sql << "SELECT id, title, tags FROM recipes WHERE id = " << recipe_id << " AND owner_user_id = " << user_id << " AND is_archived = 0;";

                    db_.query(recipe_sql.str(), [&](int argc, char** argv, char** col_names) {
                        if (argc >= 3 && argv[0] != nullptr) {
                            recipe_info["id"] = stoi(argv[0]);
                            recipe_info["title"] = argv[1] != nullptr ? argv[1] : "";
                            recipe_info["tags"] = argv[2] != nullptr ? json::parse(argv[2]) : json::array();
                        }
                        return true;
                    });

                    entry["recipe"] = recipe_info;
                }
            }

            meal_plan["entries"] = entries;
        }

        // 返回餐食计划
        response["meal_plan"] = meal_plan;

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "获取餐食计划错误: " << e.what() << endl;
    }

    return response;
}

json MealPlanService::deleteMealPlan(const string& week_start_date, int user_id) {
    json response;

    try {
        // 验证 weekStartDate 格式（YYYY-MM-DD）
        regex date_regex("\\d{4}-\\d{2}-\\d{2}");
        if (!regex_match(week_start_date, date_regex)) {
            response["errorCode"] = "INVALID_DATE_FORMAT";
            response["message"] = "日期格式不正确，需要 YYYY-MM-DD 格式";
            return response;
        }

        // 验证餐食计划是否存在
        if (!mealPlanExists(week_start_date, user_id)) {
            response["errorCode"] = "MEAL_PLAN_NOT_FOUND";
            response["message"] = "餐食计划不存在";
            return response;
        }

        // 删除餐食计划
        stringstream delete_sql;
        delete_sql << "DELETE FROM meal_plans WHERE user_id = " << user_id << " AND week_start_date = '" << week_start_date << "';";

        if (!db_.execute(delete_sql.str())) {
            response["errorCode"] = "DELETE_FAILED";
            response["message"] = "餐食计划删除失败";
            return response;
        }

        // 返回成功响应
        response["success"] = true;

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "删除餐食计划错误: " << e.what() << endl;
    }

    return response;
}

bool MealPlanService::mealPlanExists(const string& week_start_date, int user_id) {
    bool exists = false;

    stringstream select_sql;
    select_sql << "SELECT id FROM meal_plans WHERE user_id = " << user_id << " AND week_start_date = '" << week_start_date << "';";

    db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            exists = true;
        }
        return true;
    });

    return exists;
}

bool MealPlanService::validateMealPlanEntries(const json& entries, int user_id) {
    // 验证 entries 是数组
    if (!entries.is_array()) {
        return false;
    }

    // 验证每个 entry
    for (const auto& entry : entries) {
        // 验证 entry 包含必要字段
        if (!entry.contains("date") || !entry.contains("slot") || !entry.contains("recipeId")) {
            return false;
        }

        string date = entry["date"];
        string slot = entry["slot"];
        int recipe_id = entry["recipeId"];

        // 验证 date 格式（YYYY-MM-DD）
        regex date_regex("\\d{4}-\\d{2}-\\d{2}");
        if (!regex_match(date, date_regex)) {
            return false;
        }

        // 验证 slot 是有效的时段
        vector<string> valid_slots = {"breakfast", "lunch", "dinner", "snack"};
        if (find(valid_slots.begin(), valid_slots.end(), slot) == valid_slots.end()) {
            return false;
        }

        // 验证 recipeId 是有效的菜谱，且属于当前用户
        bool recipe_exists = false;
        stringstream select_sql;
        select_sql << "SELECT id FROM recipes WHERE id = " << recipe_id << " AND owner_user_id = " << user_id << " AND is_archived = 0;";

        db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 1 && argv[0] != nullptr) {
                recipe_exists = true;
            }
            return true;
        });

        if (!recipe_exists) {
            return false;
        }
    }

    return true;
}

string MealPlanService::escapeString(const string& str) {
    if (str.empty()) return str;

    sqlite3* db = db_.getConnection();
    if (db == nullptr) return str;

    char* escaped = sqlite3_mprintf("%q", str.c_str());
    string result(escaped);
    sqlite3_free(escaped);

    return result;
}
