#include "recipe_service.hpp"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <algorithm>
#include <vector>

using namespace std;

json RecipeService::createRecipe(const json& recipe_data, int user_id) {
    json response;

    try {
        // 验证必填字段
        if (!recipe_data.contains("title") || !recipe_data.contains("ingredients") || !recipe_data.contains("steps")) {
            response["errorCode"] = "MISSING_FIELDS";
            response["message"] = "缺少必填字段: title, ingredients, steps";
            return response;
        }

        string title = recipe_data["title"];
        json ingredients = recipe_data["ingredients"];
        json steps = recipe_data["steps"];

        // 验证字段长度
        if (title.empty() || ingredients.empty() || steps.empty()) {
            response["errorCode"] = "EMPTY_FIELDS";
            response["message"] = "字段不能为空";
            return response;
        }

        // 验证 ingredients 格式
        for (const auto& ingredient : ingredients) {
            if (!ingredient.contains("name") || !ingredient.contains("quantity") || !ingredient.contains("unit")) {
                response["errorCode"] = "INVALID_INGREDIENTS";
                response["message"] = "食材格式不正确，需要包含 name, quantity, unit";
                return response;
            }
        }

        // 开始事务
        if (!db_.beginTransaction()) {
            response["errorCode"] = "TRANSACTION_FAILED";
            response["message"] = "事务开始失败";
            return response;
        }

        // 准备其他字段
        string description = recipe_data.contains("description") ? recipe_data["description"] : "";
        int servings = recipe_data.contains("servings") ? recipe_data["servings"] : 1;
        json tags = recipe_data.contains("tags") ? recipe_data["tags"] : json::array();
        bool is_favorite = recipe_data.contains("is_favorite") ? recipe_data["is_favorite"] : false;

        // 插入菜谱数据
        stringstream insert_sql;
        insert_sql << "INSERT INTO recipes (owner_user_id, title, description, servings, tags, ingredients, steps, is_favorite) VALUES (" 
                   << user_id << ", '" << escapeString(title) << "', '" << escapeString(description) << "', " 
                   << servings << ", '" << escapeString(tags.dump()) << "', '" << escapeString(ingredients.dump()) << "', '" << escapeString(steps.dump()) << "', " 
                   << (is_favorite ? 1 : 0) << ");";

        if (!db_.execute(insert_sql.str())) {
            db_.rollbackTransaction();
            response["errorCode"] = "INSERT_FAILED";
            response["message"] = "菜谱创建失败";
            return response;
        }

        // 获取新创建的菜谱 ID
        int recipe_id = sqlite3_last_insert_rowid(db_.getConnection());

        // 提交事务
        if (!db_.commitTransaction()) {
            db_.rollbackTransaction();
            response["errorCode"] = "TRANSACTION_COMMIT_FAILED";
            response["message"] = "事务提交失败";
            return response;
        }

        // 返回成功响应
        response["recipeId"] = recipe_id;

    } catch (const exception& e) {
        db_.rollbackTransaction();
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "菜谱创建错误: " << e.what() << endl;
    }

    return response;
}

json RecipeService::getRecipe(int recipe_id, int user_id) {
    json response;

    try {
        // 验证菜谱是否存在且属于当前用户
        if (!recipeExistsAndBelongsToUser(recipe_id, user_id)) {
            response["errorCode"] = "RECIPE_NOT_FOUND";
            response["message"] = "菜谱不存在或不属于当前用户";
            return response;
        }

        // 查询菜谱数据
        json recipe;
        stringstream select_sql;
        select_sql << "SELECT id, title, description, servings, tags, ingredients, steps, is_favorite, created_at, updated_at FROM recipes WHERE id = " << recipe_id << ";";

        if (!db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 11 && argv[0] != nullptr) {
                recipe["id"] = stoi(argv[0]);
                recipe["title"] = argv[1] != nullptr ? argv[1] : "";
                recipe["description"] = argv[2] != nullptr ? argv[2] : "";
                recipe["servings"] = argv[3] != nullptr ? stoi(argv[3]) : 1;
                recipe["tags"] = argv[4] != nullptr ? json::parse(argv[4]) : json::array();
                recipe["ingredients"] = argv[5] != nullptr ? json::parse(argv[5]) : json::array();
                recipe["steps"] = argv[6] != nullptr ? json::parse(argv[6]) : json::array();
                recipe["is_favorite"] = argv[7] != nullptr ? (stoi(argv[7]) == 1) : false;
                recipe["created_at"] = argv[8] != nullptr ? argv[8] : "";
                recipe["updated_at"] = argv[9] != nullptr ? argv[9] : "";
            }
            return true;
        })) {
            response["errorCode"] = "QUERY_FAILED";
            response["message"] = "查询菜谱失败";
            return response;
        }

        // 返回菜谱数据
        response["recipe"] = recipe;

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "获取菜谱错误: " << e.what() << endl;
    }

    return response;
}

json RecipeService::getRecipes(int user_id, const string& keyword, 
                                const string& tag, const string& ingredient, 
                                int page, int limit) {
    json response;

    try {
        // 构建查询条件
        string conditions = buildQueryConditions(user_id, keyword, tag, ingredient);

        // 计算分页偏移量
        int offset = (page - 1) * limit;

        // 查询菜谱列表
        json recipes = json::array();
        stringstream select_sql;
        select_sql << "SELECT id, title, description, servings, tags, is_favorite, created_at, updated_at FROM recipes WHERE " << conditions 
                   << " ORDER BY created_at DESC LIMIT " << limit << " OFFSET " << offset << ";";

        if (!db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 9 && argv[0] != nullptr) {
                json recipe;
                recipe["id"] = stoi(argv[0]);
                recipe["title"] = argv[1] != nullptr ? argv[1] : "";
                recipe["description"] = argv[2] != nullptr ? argv[2] : "";
                recipe["servings"] = argv[3] != nullptr ? stoi(argv[3]) : 1;
                recipe["tags"] = argv[4] != nullptr ? json::parse(argv[4]) : json::array();
                recipe["is_favorite"] = argv[5] != nullptr ? (stoi(argv[5]) == 1) : false;
                recipe["created_at"] = argv[6] != nullptr ? argv[6] : "";
                recipe["updated_at"] = argv[7] != nullptr ? argv[7] : "";
                recipes.push_back(recipe);
            }
            return true;
        })) {
            response["errorCode"] = "QUERY_FAILED";
            response["message"] = "查询菜谱列表失败";
            return response;
        }

        // 查询总记录数
        int total = 0;
        stringstream count_sql;
        count_sql << "SELECT COUNT(*) FROM recipes WHERE " << conditions << ";";

        db_.query(count_sql.str(), [&](int argc, char** argv, char** col_names) {
            if (argc >= 1 && argv[0] != nullptr) {
                total = stoi(argv[0]);
            }
            return true;
        });

        // 计算总页数
        int total_pages = (total + limit - 1) / limit;

        // 返回菜谱列表和分页信息
        response["recipes"] = recipes;
        response["pagination"] = {
            {"page", page},
            {"limit", limit},
            {"total", total},
            {"total_pages", total_pages}
        };

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "获取菜谱列表错误: " << e.what() << endl;
    }

    return response;
}

json RecipeService::updateRecipe(int recipe_id, const json& recipe_data, int user_id) {
    json response;

    try {
        // 验证菜谱是否存在且属于当前用户
        if (!recipeExistsAndBelongsToUser(recipe_id, user_id)) {
            response["errorCode"] = "RECIPE_NOT_FOUND";
            response["message"] = "菜谱不存在或不属于当前用户";
            return response;
        }

        // 开始事务
        if (!db_.beginTransaction()) {
            response["errorCode"] = "TRANSACTION_FAILED";
            response["message"] = "事务开始失败";
            return response;
        }

        // 构建更新字段
        stringstream update_fields;
        bool first_field = true;

        if (recipe_data.contains("title")) {
            if (!first_field) update_fields << ", ";
            update_fields << "title = '" << escapeString(recipe_data["title"]) << "'";
            first_field = false;
        }

        if (recipe_data.contains("description")) {
            if (!first_field) update_fields << ", ";
            update_fields << "description = '" << escapeString(recipe_data["description"]) << "'";
            first_field = false;
        }

        if (recipe_data.contains("servings")) {
            if (!first_field) update_fields << ", ";
            update_fields << "servings = " << recipe_data["servings"];
            first_field = false;
        }

        if (recipe_data.contains("tags")) {
            if (!first_field) update_fields << ", ";
            update_fields << "tags = '" << escapeString(recipe_data["tags"].dump()) << "'";
            first_field = false;
        }

        if (recipe_data.contains("ingredients")) {
            if (!first_field) update_fields << ", ";
            update_fields << "ingredients = '" << escapeString(recipe_data["ingredients"].dump()) << "'";
            first_field = false;
        }

        if (recipe_data.contains("steps")) {
            if (!first_field) update_fields << ", ";
            update_fields << "steps = '" << escapeString(recipe_data["steps"].dump()) << "'";
            first_field = false;
        }

        if (recipe_data.contains("is_favorite")) {
            if (!first_field) update_fields << ", ";
            update_fields << "is_favorite = " << (recipe_data["is_favorite"] ? 1 : 0);
            first_field = false;
        }

        // 如果没有字段需要更新
        if (first_field) {
            response["errorCode"] = "NO_FIELDS_TO_UPDATE";
            response["message"] = "没有字段需要更新";
            return response;
        }

        // 更新菜谱数据
        stringstream update_sql;
        update_sql << "UPDATE recipes SET " << update_fields.str() << ", updated_at = CURRENT_TIMESTAMP WHERE id = " << recipe_id << ";";

        if (!db_.execute(update_sql.str())) {
            db_.rollbackTransaction();
            response["errorCode"] = "UPDATE_FAILED";
            response["message"] = "菜谱更新失败";
            return response;
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
        cerr << "更新菜谱错误: " << e.what() << endl;
    }

    return response;
}

json RecipeService::toggleFavorite(int recipe_id, bool favorite, int user_id) {
    json response;

    try {
        // 验证菜谱是否存在且属于当前用户
        if (!recipeExistsAndBelongsToUser(recipe_id, user_id)) {
            response["errorCode"] = "RECIPE_NOT_FOUND";
            response["message"] = "菜谱不存在或不属于当前用户";
            return response;
        }

        // 更新收藏状态
        stringstream update_sql;
        update_sql << "UPDATE recipes SET is_favorite = " << (favorite ? 1 : 0) << ", updated_at = CURRENT_TIMESTAMP WHERE id = " << recipe_id << ";";

        if (!db_.execute(update_sql.str())) {
            response["errorCode"] = "UPDATE_FAILED";
            response["message"] = "收藏状态更新失败";
            return response;
        }

        // 返回成功响应
        response["success"] = true;
        response["is_favorite"] = favorite;

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "更新收藏状态错误: " << e.what() << endl;
    }

    return response;
}

json RecipeService::deleteRecipe(int recipe_id, int user_id) {
    json response;

    try {
        // 验证菜谱是否存在且属于当前用户
        if (!recipeExistsAndBelongsToUser(recipe_id, user_id)) {
            response["errorCode"] = "RECIPE_NOT_FOUND";
            response["message"] = "菜谱不存在或不属于当前用户";
            return response;
        }

        // 逻辑删除菜谱（将 is_archived 置为 true）
        stringstream update_sql;
        update_sql << "UPDATE recipes SET is_archived = 1, updated_at = CURRENT_TIMESTAMP WHERE id = " << recipe_id << ";";

        if (!db_.execute(update_sql.str())) {
            response["errorCode"] = "DELETE_FAILED";
            response["message"] = "菜谱删除失败";
            return response;
        }

        // 返回成功响应
        response["success"] = true;

    } catch (const exception& e) {
        response["errorCode"] = "INTERNAL_ERROR";
        response["message"] = "内部服务器错误: " + string(e.what());
        cerr << "删除菜谱错误: " << e.what() << endl;
    }

    return response;
}

bool RecipeService::recipeExistsAndBelongsToUser(int recipe_id, int user_id) {
    bool exists = false;

    stringstream select_sql;
    select_sql << "SELECT id FROM recipes WHERE id = " << recipe_id << " AND owner_user_id = " << user_id << " AND is_archived = 0;";

    db_.query(select_sql.str(), [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            exists = true;
        }
        return true;
    });

    return exists;
}

string RecipeService::buildQueryConditions(int user_id, const string& keyword, 
                                             const string& tag, const string& ingredient) {
    vector<string> conditions;

    // 基础条件：属于当前用户且未归档
    conditions.push_back("owner_user_id = " + to_string(user_id));
    conditions.push_back("is_archived = 0");

    // 关键词搜索（标题或描述）
    if (!keyword.empty()) {
        string escaped_keyword = escapeString(keyword);
        conditions.push_back("(title LIKE '%" + escaped_keyword + "%' OR description LIKE '%" + escaped_keyword + "%')");
    }

    // 标签搜索
    if (!tag.empty()) {
        string escaped_tag = escapeString(tag);
        conditions.push_back("tags LIKE '%\"" + escaped_tag + "\"%'");
    }

    // 食材搜索
    if (!ingredient.empty()) {
        string escaped_ingredient = escapeString(ingredient);
        conditions.push_back("ingredients LIKE '%\"name\":\"" + escaped_ingredient + "\"%'");
    }

    // 组合条件
    string combined_conditions;
    for (size_t i = 0; i < conditions.size(); ++i) {
        if (i > 0) combined_conditions += " AND ";
        combined_conditions += conditions[i];
    }

    return combined_conditions;
}

string RecipeService::escapeString(const string& str) {
    if (str.empty()) return str;

    sqlite3* db = db_.getConnection();
    if (db == nullptr) return str;

    char* escaped = sqlite3_mprintf("%q", str.c_str());
    string result(escaped);
    sqlite3_free(escaped);

    return result;
}
