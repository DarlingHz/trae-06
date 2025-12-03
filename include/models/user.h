#ifndef USER_H
#define USER_H

#include <string>

class User {
private:
    int id;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string salt;
    std::string created_at;
    int question_count;
    int answer_count;

public:
    // 构造函数
    User(int id, const std::string& username, const std::string& email, 
         const std::string& password_hash, const std::string& salt, 
         const std::string& created_at, int question_count = 0, int answer_count = 0);
    
    User(const std::string& username, const std::string& email, 
         const std::string& password_hash, const std::string& salt);
    
    // Getter方法
    int getId() const;
    std::string getUsername() const;
    std::string getEmail() const;
    std::string getPasswordHash() const;
    std::string getSalt() const;
    std::string getCreatedAt() const;
    int getQuestionCount() const;
    int getAnswerCount() const;
    
    // Setter方法
    void setQuestionCount(int count);
    void setAnswerCount(int count);
    
    // 计数器方法
    void incrementQuestionCount();
    void incrementAnswerCount();
};

#endif // USER_H
