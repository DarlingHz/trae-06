#ifndef QUESTION_H
#define QUESTION_H

#include <string>

class Question {
private:
    int id;
    std::string title;
    std::string content;
    int author_id;
    std::string created_at;
    int view_count;
    int like_count;
    int answer_count;

public:
    // 构造函数
    Question(int id, const std::string& title, const std::string& content, 
             int author_id, const std::string& created_at, int view_count = 0, 
             int like_count = 0, int answer_count = 0);
    
    Question(const std::string& title, const std::string& content, int author_id);
    
    // Getter方法
    int getId() const;
    std::string getTitle() const;
    std::string getContent() const;
    int getAuthorId() const;
    std::string getCreatedAt() const;
    int getViewCount() const;
    int getLikeCount() const;
    int getAnswerCount() const;
    
    // Setter方法
    void setTitle(const std::string& title);
    void setContent(const std::string& content);
    
    // 计数器方法
    void incrementViewCount();
    void incrementLikeCount();
    void decrementLikeCount();
    void incrementAnswerCount();
    void decrementAnswerCount();
};

#endif // QUESTION_H
