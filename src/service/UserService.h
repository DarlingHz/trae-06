#ifndef USER_SERVICE_H
#define USER_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include "../model/User.h"

class UserService {
public:
    /**
     * 注册用户
     * @param user 用户对象
     * @param password 密码
     * @return 注册成功返回true，否则返回false
     */
    bool registerUser(const User& user, const std::string& password);
    
    /**
     * 用户登录
     * @param username 用户名
     * @param password 密码
     * @return 登录成功返回用户对象，否则返回nullptr
     */
    std::shared_ptr<User> login(const std::string& username, const std::string& password);
    
    /**
     * 更新用户信息
     * @param user 用户对象
     * @return 更新成功返回true，否则返回false
     */
    bool updateUserInfo(const User& user);
    
    /**
     * 更新用户密码
     * @param user_id 用户ID
     * @param old_password 旧密码
     * @param new_password 新密码
     * @return 更新成功返回true，否则返回false
     */
    bool updateUserPassword(int user_id, const std::string& old_password, const std::string& new_password);
    
    /**
     * 获取用户信息
     * @param user_id 用户ID
     * @return 获取成功返回用户对象，否则返回nullptr
     */
    std::shared_ptr<User> getUserInfo(int user_id);
    
    /**
     * 获取所有用户
     * @param page 页码
     * @param page_size 每页大小
     * @return 获取成功返回用户列表，否则返回空列表
     */
    std::vector<std::shared_ptr<User>> getAllUsers(int page, int page_size);
    
    /**
     * 获取用户总数
     * @return 获取成功返回用户总数，否则返回0
     */
    int getUserCount();
    
    /**
     * 切换用户状态
     * @param user_id 用户ID
     * @param status 用户状态
     * @return 切换成功返回true，否则返回false
     */
    bool toggleUserStatus(int user_id, const std::string& status);
    
    /**
     * 检查用户名是否存在
     * @param username 用户名
     * @return 存在返回true，否则返回false
     */
    bool checkUsernameExists(const std::string& username);
    
    /**
     * 检查邮箱是否存在
     * @param email 邮箱
     * @return 存在返回true，否则返回false
     */
    bool checkEmailExists(const std::string& email);
    
    /**
     * 生成JWT Token
     * @param user 用户对象
     * @return 生成成功返回Token，否则返回空字符串
     */
    std::string generateJWTToken(const User& user);
    
    /**
     * 验证JWT Token
     * @param token Token
     * @return 验证成功返回用户ID，否则返回-1
     */
    int verifyJWTToken(const std::string& token);
};

#endif // USER_SERVICE_H