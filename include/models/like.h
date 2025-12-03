#ifndef LIKE_H
#define LIKE_H

#include <string>

class Like {
private:
    int id;
    int user_id;
    int question_id;
    std::string created_at;

public:
    // 构造函数
    Like(int id, int user_id, int question_id, const std::string& created_at);
    Like(int user_id, int question_id);
    
    // Getter方法
    int getId() const;
    int getUserId() const;
    int getQuestionId() const;
    std::string getCreatedAt() const;
};

#endif // LIKE_H
