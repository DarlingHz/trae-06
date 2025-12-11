#ifndef BORROW_RECORD_H
#define BORROW_RECORD_H

#include <string>
#include <nlohmann/json.hpp>

class BorrowRecord {
public:
    // 构造函数
    BorrowRecord() = default;
    BorrowRecord(int id, int user_id, int book_id, const std::string& borrow_date, const std::string& due_date,
                 const std::string& return_date, const std::string& status, const std::string& created_at,
                 const std::string& updated_at);
    
    // 获取借阅记录ID
    int getId() const;
    
    // 设置借阅记录ID
    void setId(int id);
    
    // 获取用户ID
    int getUserId() const;
    
    // 设置用户ID
    void setUserId(int user_id);
    
    // 获取图书ID
    int getBookId() const;
    
    // 设置图书ID
    void setBookId(int book_id);
    
    // 获取借出时间
    std::string getBorrowDate() const;
    
    // 设置借出时间
    void setBorrowDate(const std::string& borrow_date);
    
    // 获取应还时间
    std::string getDueDate() const;
    
    // 设置应还时间
    void setDueDate(const std::string& due_date);
    
    // 获取归还时间
    std::string getReturnDate() const;
    
    // 设置归还时间
    void setReturnDate(const std::string& return_date);
    
    // 获取状态
    std::string getStatus() const;
    
    // 设置状态
    void setStatus(const std::string& status);
    
    // 获取创建时间
    std::string getCreatedAt() const;
    
    // 设置创建时间
    void setCreatedAt(const std::string& created_at);
    
    // 获取更新时间
    std::string getUpdatedAt() const;
    
    // 设置更新时间
    void setUpdatedAt(const std::string& updated_at);
    
    // 转换为JSON对象
    nlohmann::json toJson() const;
    
    // 从JSON对象转换
    static BorrowRecord fromJson(const nlohmann::json& json_obj);
    
private:
    int id_ = 0;
    int user_id_ = 0;
    int book_id_ = 0;
    std::string borrow_date_;
    std::string due_date_;
    std::string return_date_;
    std::string status_ = "borrowed";
    std::string created_at_;
    std::string updated_at_;
};

#endif // BORROW_RECORD_H