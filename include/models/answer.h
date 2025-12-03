#ifndef ANSWER_H
#define ANSWER_H

#include <string>

class Answer {
private:
    int id;
    std::string content;
    int author_id;
    int question_id;
    std::string created_at;
    bool is_accepted;

public:
    // 构造函数
    Answer(int id, const std::string& content, int author_id, int question_id, 
           const std::string& created_at, bool is_accepted = false);
    
    Answer(const std::string& content, int author_id, int question_id);
    
    // Getter方法
    int getId() const;
    std::string getContent() const;
    int getAuthorId() const;
    int getQuestionId() const;
    std::string getCreatedAt() const;
    bool isAccepted() const;
    
    // Setter方法
    void setContent(const std::string& content);
    void setIsAccepted(bool accepted);
};

#endif // ANSWER_H
