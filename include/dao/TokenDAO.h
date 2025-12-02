#pragma once

#include <string>
#include <optional>
#include "BaseDAO.h"
#include "models/Token.h"

namespace pet_hospital {

class TokenDAO : public BaseDAO {
public:
    TokenDAO() = default;
    ~TokenDAO() override = default;

    // 创建token
    bool create_token(const Token& token);

    // 根据token值查询token
    std::optional<Token> get_token_by_value(const std::string& token);

    // 根据用户ID查询token
    std::optional<Token> get_token_by_user_id(int user_id);

    // 更新token
    bool update_token(const Token& token);

    // 删除token
    bool delete_token(int token_id);

    // 根据token值删除token
    bool delete_token_by_value(const std::string& token);
};

} // namespace pet_hospital