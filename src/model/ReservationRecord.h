#ifndef RESERVATION_RECORD_H
#define RESERVATION_RECORD_H

#include <string>
#include <nlohmann/json.hpp>

class ReservationRecord {
public:
    // 构造函数
    ReservationRecord() = default;
    ReservationRecord(int id, int user_id, int book_id, const std::string& reservation_date, const std::string& status,
                      const std::string& confirmed_date, const std::string& expire_date, int queue_position,
                      const std::string& created_at, const std::string& updated_at);
    
    // 获取预约记录ID
    int getId() const;
    
    // 设置预约记录ID
    void setId(int id);
    
    // 获取用户ID
    int getUserId() const;
    
    // 设置用户ID
    void setUserId(int user_id);
    
    // 获取图书ID
    int getBookId() const;
    
    // 设置图书ID
    void setBookId(int book_id);
    
    // 获取预约时间
    std::string getReservationDate() const;
    
    // 设置预约时间
    void setReservationDate(const std::string& reservation_date);
    
    // 获取状态
    std::string getStatus() const;
    
    // 设置状态
    void setStatus(const std::string& status);
    
    // 获取确认时间
    std::string getConfirmedDate() const;
    
    // 设置确认时间
    void setConfirmedDate(const std::string& confirmed_date);
    
    // 获取过期时间
    std::string getExpireDate() const;
    
    // 设置过期时间
    void setExpireDate(const std::string& expire_date);
    
    // 获取排队位置
    int getQueuePosition() const;
    
    // 设置排队位置
    void setQueuePosition(int queue_position);
    
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
    static ReservationRecord fromJson(const nlohmann::json& json_obj);
    
private:
    int id_ = 0;
    int user_id_ = 0;
    int book_id_ = 0;
    std::string reservation_date_;
    std::string status_ = "pending";
    std::string confirmed_date_;
    std::string expire_date_;
    int queue_position_ = 0;
    std::string created_at_;
    std::string updated_at_;
};

#endif // RESERVATION_RECORD_H