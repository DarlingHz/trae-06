#include <crow.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include "database.hpp"
#include "user_service.hpp"
#include "recipe_service.hpp"
#include "meal_plan_service.hpp"
#include "shopping_list_service.hpp"

using namespace std;
using namespace crow;
using json = nlohmann::json;

// 全局服务实例
UserService user_service;
RecipeService recipe_service;
MealPlanService meal_plan_service;
ShoppingListService shopping_list_service;

// 验证用户token的中间件
struct AuthMiddleware {
    void before_handle(request& req, response& res, context& ctx) {
        // 检查是否是不需要认证的路由
        string path = req.url;        if (path == "/api/users/register" || path == "/api/users/login") {
            return;
        }

        // 检查请求头中是否包含X-Auth-Token
        auto token_it = req.headers.find("X-Auth-Token");
        if (token_it == req.headers.end()) {
            res.code = 401;
            res.body = json({{"errorCode", "UNAUTHORIZED"}, {"message", "缺少认证token"}}).dump();
            res.end();
            return;
        }

        // 验证token
        string token = token_it->second;
        json verify_result = user_service.verifyToken(token);

        if (!verify_result.contains("success") || !verify_result["success"]) {
            res.code = 401;
            res.body = json({{"errorCode", "UNAUTHORIZED"}, {"message", "无效的认证token"}}).dump();
            res.end();
            return;
        }

        // 将用户ID存储到上下文
        ctx["user_id"] = verify_result["user_id"];
    }

    void after_handle(request& req, response& res, context& ctx) {
        // 可以在这里添加日志记录等功能
    }
};

int main() {
    try {
        // 初始化数据库
        Database& db = Database::getInstance();
        if (!db.initialize("data/meal_plan_manager.db")) {
            cerr << "数据库初始化失败" << endl;
            return 1;
        }

        // 创建Crow应用实例
        App<AuthMiddleware> app;

        // 用户注册路由
        CROW_ROUTE(app, "/api/users/register").methods(HTTPMethod::POST)([](const request& req) {
            response res;
            try {
                json request_body = json::parse(req.body);
                json result = user_service.registerUser(request_body);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 201;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "用户注册错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 用户登录路由
        CROW_ROUTE(app, "/api/users/login").methods(HTTPMethod::POST)([](const request& req) {
            response res;
            try {
                json request_body = json::parse(req.body);
                json result = user_service.loginUser(request_body);

                if (result.contains("errorCode")) {
                    res.code = 401;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "用户登录错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 创建菜谱路由
        CROW_ROUTE(app, "/api/recipes").methods(HTTPMethod::POST)([](const request& req, context& ctx) {
            response res;
            try {
                json request_body = json::parse(req.body);
                int user_id = ctx["user_id"];
                json result = recipe_service.createRecipe(request_body, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 201;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "创建菜谱错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 获取菜谱列表路由
        CROW_ROUTE(app, "/api/recipes").methods(HTTPMethod::GET)([](const request& req, context& ctx) {
            response res;
            try {
                string keyword = req.url_params.get("keyword", "");
                string tag = req.url_params.get("tag", "");
                string ingredient = req.url_params.get("ingredient", "");
                int user_id = ctx["user_id"];
                json result = recipe_service.getRecipes(keyword, tag, ingredient, user_id);

                res.code = 200;
                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "获取菜谱列表错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 获取单个菜谱路由
        CROW_ROUTE(app, "/api/recipes/<int>").methods(HTTPMethod::GET)([](const request& req, context& ctx, int recipe_id) {
            response res;
            try {
                int user_id = ctx["user_id"];
                json result = recipe_service.getRecipe(recipe_id, user_id);

                if (result.contains("errorCode")) {
                    res.code = 404;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "获取单个菜谱错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 更新菜谱路由
        CROW_ROUTE(app, "/api/recipes/<int>").methods(HTTPMethod::PUT)([](const request& req, context& ctx, int recipe_id) {
            response res;
            try {
                json request_body = json::parse(req.body);
                int user_id = ctx["user_id"];
                json result = recipe_service.updateRecipe(recipe_id, request_body, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "更新菜谱错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 标记菜谱为收藏路由
        CROW_ROUTE(app, "/api/recipes/<int>/favorite").methods(HTTPMethod::POST)([](const request& req, context& ctx, int recipe_id) {
            response res;
            try {
                json request_body = json::parse(req.body);
                int user_id = ctx["user_id"];
                json result = recipe_service.toggleFavorite(recipe_id, request_body, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "标记菜谱为收藏错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 删除菜谱路由
        CROW_ROUTE(app, "/api/recipes/<int>").methods(HTTPMethod::DELETE)([](const request& req, context& ctx, int recipe_id) {
            response res;
            try {
                int user_id = ctx["user_id"];
                json result = recipe_service.deleteRecipe(recipe_id, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "删除菜谱错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 创建或更新餐食计划路由
        CROW_ROUTE(app, "/api/meal-plans").methods(HTTPMethod::POST)([](const request& req, context& ctx) {
            response res;
            try {
                json request_body = json::parse(req.body);
                int user_id = ctx["user_id"];
                json result = meal_plan_service.createOrUpdateMealPlan(request_body, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "创建或更新餐食计划错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 获取餐食计划路由
        CROW_ROUTE(app, "/api/meal-plans").methods(HTTPMethod::GET)([](const request& req, context& ctx) {
            response res;
            try {
                string week_start_date = req.url_params.get("weekStartDate", "");
                int user_id = ctx["user_id"];
                json result = meal_plan_service.getMealPlan(week_start_date, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "获取餐食计划错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 删除餐食计划路由
        CROW_ROUTE(app, "/api/meal-plans").methods(HTTPMethod::DELETE)([](const request& req, context& ctx) {
            response res;
            try {
                string week_start_date = req.url_params.get("weekStartDate", "");
                int user_id = ctx["user_id"];
                json result = meal_plan_service.deleteMealPlan(week_start_date, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "删除餐食计划错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 生成购物清单路由
        CROW_ROUTE(app, "/api/meal-plans/shopping-list").methods(HTTPMethod::GET)([](const request& req, context& ctx) {
            response res;
            try {
                string from_date = req.url_params.get("from", "");
                string to_date = req.url_params.get("to", "");
                int user_id = ctx["user_id"];
                json result = shopping_list_service.generateShoppingList(from_date, to_date, user_id);

                if (result.contains("errorCode")) {
                    res.code = 400;
                } else {
                    res.code = 200;
                }

                res.body = result.dump();
            } catch (const exception& e) {
                res.code = 500;
                res.body = json({{"errorCode", "INTERNAL_ERROR"}, {"message", "内部服务器错误: " + string(e.what())}}).dump();
                cerr << "生成购物清单错误: " << e.what() << endl;
            }

            res.set_header("Content-Type", "application/json");
            return res;
        });

        // 启动服务器
        cout << "餐食计划管理系统服务启动成功，监听端口: 8080" << endl;
        app.port(8080).multithreaded().run();

    } catch (const exception& e) {
        cerr << "服务启动失败: " << e.what() << endl;
        return 1;
    }

    return 0;
}
