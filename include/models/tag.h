#ifndef TAG_H
#define TAG_H

#include <string>

class Tag {
private:
    int id;
    std::string name;
    int question_count;

public:
    // 构造函数
    Tag(int id, const std::string& name, int question_count = 0);
    Tag(const std::string& name);
    
    // Getter方法
    int getId() const;
    std::string getName() const;
    int getQuestionCount() const;
    
    // Setter方法
    void setQuestionCount(int count);
    
    // 计数器方法
    void incrementQuestionCount();
    void decrementQuestionCount();
};

#endif // TAG_H
