#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>
#include "shopping_list_service.hpp"
#include "database.hpp"

using json = nlohmann::json;

// 测试数据库连接
TEST_CASE("Database Connection Test", "[database]") {
    Database& db = Database::getInstance();
    REQUIRE(db.initialize("test.db") == true);
}

// 测试购物清单生成功能
TEST_CASE("Shopping List Generation Test", "[shopping_list]") {
    // 初始化测试数据库
    Database& db = Database::getInstance();
    REQUIRE(db.initialize("test.db") == true);

    // 创建测试用户
    string create_user_sql = R"(
        INSERT INTO users (name, email, password) VALUES ('Test User', 'test@example.com', 'password123');
    )";
    REQUIRE(db.execute(create_user_sql) == true);

    // 获取测试用户ID
    int user_id = 0;
    string select_user_sql = "SELECT id FROM users WHERE email = 'test@example.com';";
    db.query(select_user_sql, [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            user_id = stoi(argv[0]);
        }
        return true;
    });
    REQUIRE(user_id > 0);

    // 创建测试菜谱1
    json ingredients1 = json::array();
    ingredients1.push_back({{"name", "鸡蛋"}, {"quantity", 2}, {"unit", "个"}});
    ingredients1.push_back({{"name", "牛奶"}, {"quantity", 250}, {"unit", "ml"}});

    string create_recipe1_sql = R"(
        INSERT INTO recipes (owner_user_id, title, description, servings, tags, ingredients, steps) 
        VALUES (""" + to_string(user_id) + """, '煎蛋', '简单的煎蛋', 1, '["早餐","快手菜"]', '""" + ingredients1.dump() + """', '["1. 热锅","2. 打入鸡蛋","3. 煎至两面金黄"]');
    )";
    REQUIRE(db.execute(create_recipe1_sql) == true);

    // 创建测试菜谱2
    json ingredients2 = json::array();
    ingredients2.push_back({{"name", "鸡蛋"}, {"quantity", 1}, {"unit", "个"}});
    ingredients2.push_back({{"name", "面粉"}, {"quantity", 100}, {"unit", "克"}});
    ingredients2.push_back({{"name", "牛奶"}, {"quantity", 150}, {"unit", "ml"}});

    string create_recipe2_sql = R"(
        INSERT INTO recipes (owner_user_id, title, description, servings, tags, ingredients, steps) 
        VALUES (""" + to_string(user_id) + """, '煎饼', '简单的煎饼', 1, '["早餐","快手菜"]', '""" + ingredients2.dump() + """', '["1. 混合面粉和鸡蛋","2. 加入牛奶","3. 煎至两面金黄"]');
    )";
    REQUIRE(db.execute(create_recipe2_sql) == true);

    // 获取测试菜谱ID
    int recipe1_id = 0;
    int recipe2_id = 0;

    string select_recipe1_sql = "SELECT id FROM recipes WHERE title = '煎蛋' AND owner_user_id = " + to_string(user_id) + ";";
    db.query(select_recipe1_sql, [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            recipe1_id = stoi(argv[0]);
        }
        return true;
    });

    string select_recipe2_sql = "SELECT id FROM recipes WHERE title = '煎饼' AND owner_user_id = " + to_string(user_id) + ";";
    db.query(select_recipe2_sql, [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            recipe2_id = stoi(argv[0]);
        }
        return true;
    });

    REQUIRE(recipe1_id > 0);
    REQUIRE(recipe2_id > 0);

    // 创建测试餐食计划
    json entries = json::array();
    entries.push_back({{"date", "2025-01-06"}, {"slot", "breakfast"}, {"recipeId", recipe1_id}});
    entries.push_back({{"date", "2025-01-07"}, {"slot", "breakfast"}, {"recipeId", recipe2_id}});

    string create_meal_plan_sql = R"(
        INSERT INTO meal_plans (user_id, week_start_date, entries) 
        VALUES (""" + to_string(user_id) + """, '2025-01-06', '""" + entries.dump() + """');
    )";
    REQUIRE(db.execute(create_meal_plan_sql) == true);

    // 测试购物清单生成
    ShoppingListService shopping_list_service;
    json result = shopping_list_service.generateShoppingList("2025-01-06", "2025-01-07", user_id);

    REQUIRE(result.contains("shopping_list"));
    REQUIRE(result["shopping_list"].is_array());
    REQUIRE(result["shopping_list"].size() == 3);

    // 验证食材聚合结果
    unordered_map<string, pair<double, string>> expected_ingredients;
    expected_ingredients["鸡蛋|个"] = make_pair(3.0, "个");
    expected_ingredients["牛奶|ml"] = make_pair(400.0, "ml");
    expected_ingredients["面粉|克"] = make_pair(100.0, "克");

    for (const auto& ingredient : result["shopping_list"]) {
        string key = ingredient["name"] + "|" + ingredient["unit"];
        REQUIRE(expected_ingredients.find(key) != expected_ingredients.end());
        REQUIRE(ingredient["quantity"] == expected_ingredients[key].first);
        REQUIRE(ingredient["unit"] == expected_ingredients[key].second);
    }

    // 清理测试数据
    string delete_meal_plan_sql = "DELETE FROM meal_plans WHERE user_id = " + to_string(user_id) + ";";
    string delete_recipe_sql = "DELETE FROM recipes WHERE owner_user_id = " + to_string(user_id) + ";";
    string delete_user_sql = "DELETE FROM users WHERE id = " + to_string(user_id) + ";";

    db.execute(delete_meal_plan_sql);
    db.execute(delete_recipe_sql);
    db.execute(delete_user_sql);
}

// 测试边界情况：没有餐食计划
TEST_CASE("Shopping List Generation with No Meal Plans Test", "[shopping_list]") {
    // 初始化测试数据库
    Database& db = Database::getInstance();
    REQUIRE(db.initialize("test.db") == true);

    // 创建测试用户
    string create_user_sql = R"(
        INSERT INTO users (name, email, password) VALUES ('Test User', 'test@example.com', 'password123');
    )";
    REQUIRE(db.execute(create_user_sql) == true);

    // 获取测试用户ID
    int user_id = 0;
    string select_user_sql = "SELECT id FROM users WHERE email = 'test@example.com';";
    db.query(select_user_sql, [&](int argc, char** argv, char** col_names) {
        if (argc >= 1 && argv[0] != nullptr) {
            user_id = stoi(argv[0]);
        }
        return true;
    });
    REQUIRE(user_id > 0);

    // 测试购物清单生成
    ShoppingListService shopping_list_service;
    json result = shopping_list_service.generateShoppingList("2025-01-06", "2025-01-07", user_id);

    REQUIRE(result.contains("shopping_list"));
    REQUIRE(result["shopping_list"].is_array());
    REQUIRE(result["shopping_list"].size() == 0);

    // 清理测试数据
    string delete_user_sql = "DELETE FROM users WHERE id = " + to_string(user_id) + ";";
    db.execute(delete_user_sql);
}
