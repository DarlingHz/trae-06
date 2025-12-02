#include "shopping_list_service.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <regex>
#include <unordered_map>

using namespace std;

json ShoppingListService::generateShoppingList(const string& from_date, const string& to_date, int user_id) {
    json response;

    try {
        // 验证日期格式
        if (!validateDate(from_date) || !validateDate(to_date)) {
            response["errorCode"] = "INVALID_DATE_FORMAT";
            response["message"] = "日期格式不正确，需要 YYYY-MM-DD 格式";
            return response;
        }

        // 查询指定日期区间内的所有餐食计划
        vector<int> recipe_ids;
        stringstream select_meal_plans_sql;
        select_meal_plans_sql << "SELECT entries FROM meal_plans WHERE user_id = " << user_id << " AND week_start_date <= '" << to_date << "' AND date(week_start_date, '+6 days') >= '" << from_date << "';";

        if (!db_.query(select_meal_plans_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 1 && argv[0] != nullptr) {
                json entries = json::parse(argv[0]);
                for (const auto& entry : entries) {
                    if (entry.contains("recipeId")) {
                        int recipe_id = entry["recipeId"];
                        // 检查日期是否在指定区间内
                        string entry_date = entry["date"];
                        if (entry_date >= from_date && entry_date <= to_date) {
                            recipe_ids.push_back(recipe_id);
                        }
                    }
                }
            }
            return true;
        })) {
            response["errorCode"] = "QUERY_FAILED";
            response["message"] = "查询餐食计划失败";
            return response;
        }

        if (recipe_ids.empty()) {
            response["shopping_list"] = json::array();
            return response;
        }

        // 查询这些菜谱的食材信息
        unordered_map<string, pair<double, string>> shopping_list_map;
        stringstream select_recipes_sql;
        select_recipes_sql << "SELECT ingredients FROM recipes WHERE id IN (";
        for (size_t i = 0; i < recipe_ids.size(); ++i) {
            if (i > 0) {
                select_recipes_sql << ", ";
            }
            select_recipes_sql << recipe_ids[i];
        }
        select_recipes_sql << ") AND owner_user_id = " << user_id << " AND is_archived = 0;";

        if (!db_.query(select_recipes_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 1 && argv[0] != nullptr) {
                json ingredients = json::parse(argv[0]);
                for (const auto& ingredient : ingredients) {
                    if (ingredient.contains("name") && ingredient.contains("quantity") && ingredient.contains("unit")) {
                        string name = ingredient["name"];
                        double quantity = ingredient["quantity"];
                        string unit = ingredient["unit"];

                        // 生成唯一键（名称 + 单位）
                        string key = name + "|" + unit;

                        // 聚合数量
                        if (shopping_list_map.find(key) != shopping_list_map.end()) {
                            shopping_list_map[key].first += quantity;
                        } else {
                            shopping_list_map[key] = make_pair(quantity, unit);
                        }
                    }
                }
            }
            return true;
        })) {
            response["errorCode"] = "QUERY_FAILED";
            response["message"] = "查询菜谱食材失败";
            return response;
        }

        // 转换为数组格式
        json shopping_list;
        for (const auto& item : shopping_list_map) {
            json ingredient;
            size_t separator_pos = item.first.find("|");
            ingredient["name"] = item.first.substr(0, separator_pos);
            ingredient["quantity"] = item.second.first;
            ingredient["unit"] = item.second.second;
            shopping_list.push_back(ingredient);
        }

        // 返回购物清单
        response["shopping_list"] = shopping_list;

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "生成购物清单错误: " << e.what() << endl;
    }

    return response;
}

bool ShoppingListService::validateDate(const string& date) {
    // 验证日期格式（YYYY-MM-DD）
    regex date_regex("\\d{4}-\\d{2}-\\d{2}");
    return regex_match(date, date_regex);
}

string ShoppingListService::escapeString(const string& str) {
    if (str.empty()) return str;

    sqlite3* db = db_.getConnection();
    if (db == nullptr) return str;

    char* escaped = sqlite3_mprintf("%q", str.c_str());
    string result(escaped);
    sqlite3_free(escaped);

    return result;
}
