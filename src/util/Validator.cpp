#include "Validator.h"
#include <string>
#include <regex>
#include <sstream>

namespace library {
namespace util {

bool Validator::isValidUsername(const std::string& username) {
    // 用户名长度在6-20个字符之间，只能包含字母、数字和下划线
    std::regex pattern("^[a-zA-Z0-9_]{6,20}$");
    return std::regex_match(username, pattern);
}

bool Validator::isValidPassword(const std::string& password) {
    // 密码长度在8-20个字符之间，必须包含字母和数字
    if (password.length() < 8 || password.length() > 20) {
        return false;
    }
    
    bool has_letter = false;
    bool has_digit = false;
    
    for (char c : password) {
        if (std::isalpha(c)) {
            has_letter = true;
        } else if (std::isdigit(c)) {
            has_digit = true;
        }
        
        if (has_letter && has_digit) {
            break;
        }
    }
    
    return has_letter && has_digit;
}

bool Validator::isValidEmail(const std::string& email) {
    // 邮箱格式验证
    std::regex pattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, pattern);
}

bool Validator::isValidPhone(const std::string& phone) {
    // 手机号格式验证：11位数字，以1开头
    std::regex pattern("^1[0-9]{10}$");
    return std::regex_match(phone, pattern);
}

bool Validator::isValidISBN(const std::string& isbn) {
    // ISBN格式验证：支持ISBN-10和ISBN-13
    std::string cleaned_isbn;
    
    // 移除所有非数字字符
    for (char c : isbn) {
        if (std::isdigit(c) || c == 'X' || c == 'x') {
            cleaned_isbn += c;
        }
    }
    
    // 检查长度
    if (cleaned_isbn.length() != 10 && cleaned_isbn.length() != 13) {
        return false;
    }
    
    // ISBN-10验证
    if (cleaned_isbn.length() == 10) {
        int sum = 0;
        for (int i = 0; i < 9; ++i) {
            sum += (cleaned_isbn[i] - '0') * (10 - i);
        }
        
        char last_char = cleaned_isbn[9];
        int last_digit;
        
        if (last_char == 'X' || last_char == 'x') {
            last_digit = 10;
        } else {
            last_digit = last_char - '0';
        }
        
        sum += last_digit;
        
        return sum % 11 == 0;
    }
    
    // ISBN-13验证
    if (cleaned_isbn.length() == 13) {
        int sum = 0;
        for (int i = 0; i < 12; ++i) {
            int digit = cleaned_isbn[i] - '0';
            sum += (i % 2 == 0) ? digit : digit * 3;
        }
        
        int check_digit = cleaned_isbn[12] - '0';
        int calculated_check_digit = (10 - (sum % 10)) % 10;
        
        return check_digit == calculated_check_digit;
    }
    
    return false;
}

bool Validator::isValidDate(const std::string& date) {
    // 日期格式验证：YYYY-MM-DD
    std::regex pattern("^\\d{4}-\\d{2}-\\d{2}$");
    
    if (!std::regex_match(date, pattern)) {
        return false;
    }
    
    // 检查日期是否有效
    int year, month, day;
    char dash1, dash2;
    
    std::istringstream ss(date);
    ss >> year >> dash1 >> month >> dash2 >> day;
    
    if (dash1 != '-' || dash2 != '-') {
        return false;
    }
    
    // 检查年份
    if (year < 1900 || year > 2100) {
        return false;
    }
    
    // 检查月份
    if (month < 1 || month > 12) {
        return false;
    }
    
    // 检查天数
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // 检查闰年
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        days_in_month[1] = 29;
    }
    
    if (day < 1 || day > days_in_month[month - 1]) {
        return false;
    }
    
    return true;
}

bool Validator::isValidTitle(const std::string& title) {
    // 书名长度在1-100个字符之间
    return title.length() >= 1 && title.length() <= 100;
}

bool Validator::isValidAuthor(const std::string& author) {
    // 作者长度在1-50个字符之间
    return author.length() >= 1 && author.length() <= 50;
}

bool Validator::isValidToken(const std::string& token) {
    // token格式验证：非空字符串
    return !token.empty();
}

int Validator::parseToken(const std::string& token) {
    // 解析token获取用户ID
    // token格式为"user_id:token_string"
    size_t pos = token.find(':');
    
    if (pos == std::string::npos) {
        return -1;
    }
    
    std::string user_id_str = token.substr(0, pos);
    
    try {
        int user_id = std::stoi(user_id_str);
        return user_id;
    } catch (const std::invalid_argument& e) {
        return -1;
    } catch (const std::out_of_range& e) {
        return -1;
    }
}

} // namespace util
} // namespace library