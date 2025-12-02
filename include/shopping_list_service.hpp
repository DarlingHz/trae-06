#ifndef SHOPPING_LIST_SERVICE_HPP
#define SHOPPING_LIST_SERVICE_HPP

#include <string>
#include <nlohmann/json.hpp>
#include "database.hpp"

using json = nlohmann::json;

class ShoppingListService {
public:
    ShoppingListService() : db_(Database::getInstance()) {}

    // 生成购物清单
    json generateShoppingList(const std::string& from_date, const std::string& to_date, int user_id);

private:
    Database& db_;

    // 验证日期格式
    bool validateDate(const std::string& date);

    // 转义字符串以防止 SQL 注入
    std::string escapeString(const std::string& str);
};

#endif // SHOPPING_LIST_SERVICE_HPP
