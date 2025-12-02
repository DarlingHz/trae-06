#ifndef RECIPE_SERVICE_HPP
#define RECIPE_SERVICE_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "database.hpp"

using json = nlohmann::json;

class RecipeService {
public:
    RecipeService() : db_(Database::getInstance()) {}

    // 创建菜谱
    json createRecipe(const json& recipe_data, int user_id);

    // 获取特定菜谱
    json getRecipe(int recipe_id, int user_id);

    // 获取菜谱列表
    json getRecipes(int user_id, const std::string& keyword = "", 
                    const std::string& tag = "", const std::string& ingredient = "", 
                    int page = 1, int limit = 10);

    // 更新菜谱
    json updateRecipe(int recipe_id, const json& recipe_data, int user_id);

    // 标记菜谱为收藏或取消收藏
    json toggleFavorite(int recipe_id, bool favorite, int user_id);

    // 删除菜谱（逻辑删除）
    json deleteRecipe(int recipe_id, int user_id);

private:
    Database& db_;

    // 验证菜谱是否存在且属于当前用户
    bool recipeExistsAndBelongsToUser(int recipe_id, int user_id);

    // 构建查询条件
    std::string buildQueryConditions(int user_id, const std::string& keyword, 
                                      const std::string& tag, const std::string& ingredient);

    // 转义字符串以防止 SQL 注入
    std::string escapeString(const std::string& str);
};

#endif // RECIPE_SERVICE_HPP
