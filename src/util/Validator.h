#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
#include <regex>

namespace library {
namespace util {

class Validator {
public:
    /**
     * 验证用户名格式
     * 要求：长度在6-20个字符之间，只能包含字母、数字和下划线
     */
    static bool isValidUsername(const std::string& username);
    
    /**
     * 验证密码格式
     * 要求：长度在8-20个字符之间，必须包含字母和数字
     */
    static bool isValidPassword(const std::string& password);
    
    /**
     * 验证邮箱格式
     */
    static bool isValidEmail(const std::string& email);
    
    /**
     * 验证手机号格式
     * 要求：11位数字，以1开头
     */
    static bool isValidPhone(const std::string& phone);
    
    /**
     * 验证ISBN格式
     * 支持ISBN-10和ISBN-13格式
     */
    static bool isValidISBN(const std::string& isbn);
    
    /**
     * 验证日期格式
     * 要求：YYYY-MM-DD格式
     */
    static bool isValidDate(const std::string& date);
    
    /**
     * 验证书名格式
     * 要求：长度在1-100个字符之间
     */
    static bool isValidTitle(const std::string& title);
    
    /**
     * 验证作者格式
     * 要求：长度在1-50个字符之间
     */
    static bool isValidAuthor(const std::string& author);
    
    /**
     * 验证token格式
     * 要求：非空字符串
     */
    static bool isValidToken(const std::string& token);
    
    /**
     * 解析token获取用户ID
     * 要求：token格式为"user_id:token_string"
     * 返回：用户ID，如果解析失败返回-1
     */
    static int parseToken(const std::string& token);
};

} // namespace util
} // namespace library

#endif // VALIDATOR_H