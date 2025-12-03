#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <string>
#include <cstdlib>
#include <stdexcept>

/**
 * @brief 创建临时测试数据库路径
 * @return 临时数据库路径
 */
inline std::string create_temp_db_path() {
    char* temp_dir = std::getenv("TMPDIR");
    if (!temp_dir) {
        temp_dir = const_cast<char*>("/tmp");
    }
    return std::string(temp_dir) + "/test_announcements.db";
}

/**
 * @brief 删除临时测试数据库
 * @param db_path 数据库路径
 */
inline void delete_temp_db(const std::string& db_path) {
    std::remove(db_path.c_str());
}

/**
 * @brief 测试断言宏
 */
#define ASSERT_TRUE(condition) 
    if (!(condition)) { 
        throw std::runtime_error("Assertion failed: " #condition); 
    }

#define ASSERT_FALSE(condition) 
    if ((condition)) { 
        throw std::runtime_error("Assertion failed: !" #condition); 
    }

#define ASSERT_EQUAL(expected, actual) 
    if ((expected) != (actual)) { 
        throw std::runtime_error("Assertion failed: " #expected " == " #actual); 
    }

#define ASSERT_NOT_EQUAL(expected, actual) 
    if ((expected) == (actual)) { 
        throw std::runtime_error("Assertion failed: " #expected " != " #actual); 
    }

#define ASSERT_THROWS(expression) 
    bool threw = false; 
    try { 
        (expression); 
    } catch (...) { 
        threw = true; 
    } 
    if (!threw) { 
        throw std::runtime_error("Assertion failed: " #expression " should throw an exception"); 
    }

#endif // TEST_UTILS_H
