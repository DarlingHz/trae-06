#ifndef BOOK_H
#define BOOK_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class Book {
public:
    // 构造函数
    Book() = default;
    Book(int id, const std::string& title, const std::string& author, const std::string& isbn,
         const std::string& description, int total_copies, int available_copies, int borrowed_copies,
         const std::string& status, const std::string& created_at, const std::string& updated_at);
    
    // 获取图书ID
    int getId() const;
    
    // 设置图书ID
    void setId(int id);
    
    // 获取书名
    std::string getTitle() const;
    
    // 设置书名
    void setTitle(const std::string& title);
    
    // 获取作者
    std::string getAuthor() const;
    
    // 设置作者
    void setAuthor(const std::string& author);
    
    // 获取ISBN
    std::string getIsbn() const;
    
    // 设置ISBN
    void setIsbn(const std::string& isbn);
    
    // 获取简介
    std::string getDescription() const;
    
    // 设置简介
    void setDescription(const std::string& description);
    
    // 获取总册数
    int getTotalCopies() const;
    
    // 设置总册数
    void setTotalCopies(int total_copies);
    
    // 获取可借数量
    int getAvailableCopies() const;
    
    // 设置可借数量
    void setAvailableCopies(int available_copies);
    
    // 获取在借数量
    int getBorrowedCopies() const;
    
    // 设置在借数量
    void setBorrowedCopies(int borrowed_copies);
    
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
    
    // 获取分类
    std::vector<std::string> getCategories() const;
    
    // 设置分类
    void setCategories(const std::vector<std::string>& categories);
    
    // 转换为JSON对象
    nlohmann::json toJson() const;
    
    // 从JSON对象转换
    static Book fromJson(const nlohmann::json& json_obj);
    
private:
    int id_ = 0;
    std::string title_;
    std::string author_;
    std::string isbn_;
    std::string description_;
    int total_copies_ = 0;
    int available_copies_ = 0;
    int borrowed_copies_ = 0;
    std::string status_ = "active";
    std::string created_at_;
    std::string updated_at_;
    std::vector<std::string> categories_;
};

#endif // BOOK_H