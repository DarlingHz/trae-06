#ifndef PARKING_DAO_H
#define PARKING_DAO_H

#include "models.h"
#include <vector>
#include <optional>
#include <string>

// 用户数据访问接口
class UserDAO {
public:
    virtual ~UserDAO() = default;

    // 创建用户
    virtual int create(const User& user) = 0;

    // 根据ID查找用户
    virtual std::optional<User> find_by_id(int id) const = 0;

    // 根据邮箱查找用户
    virtual std::optional<User> find_by_email(const std::string& email) const = 0;

    // 更新用户信息
    virtual bool update(const User& user) = 0;
};

// 停车位数据访问接口
class ParkingSpotDAO {
public:
    virtual ~ParkingSpotDAO() = default;

    // 创建停车位
    virtual int create(const ParkingSpot& spot) = 0;

    // 根据ID查找停车位
    virtual std::optional<ParkingSpot> find_by_id(int id) const = 0;

    // 根据用户ID查找所有停车位
    virtual std::vector<ParkingSpot> find_by_owner(int user_id) const = 0;

    // 搜索可用停车位（按时间段和地址）
    virtual std::vector<ParkingSpot> search_available(
        const std::string& city, std::time_t start_time, std::time_t end_time) const = 0;

    // 更新停车位
    virtual bool update(const ParkingSpot& spot) = 0;

    // 检查停车位在指定时间段是否可用
    virtual bool is_available(int spot_id, std::time_t start_time, std::time_t end_time) const = 0;
};

// 预约数据访问接口
class ReservationDAO {
public:
    virtual ~ReservationDAO() = default;

    // 创建预约
    virtual int create(const Reservation& reservation) = 0;

    // 根据ID查找预约
    virtual std::optional<Reservation> find_by_id(int id) const = 0;

    // 根据租客ID查找所有预约
    virtual std::vector<Reservation> find_by_renter(int renter_id) const = 0;

    // 根据车位ID查找所有预约
    virtual std::vector<Reservation> find_by_spot(int spot_id) const = 0;

    // 根据车主ID查找所有预约（收到的预约）
    virtual std::vector<Reservation> find_by_owner(int owner_id) const = 0;

    // 更新预约状态
    virtual bool update_status(int id, ReservationStatus status) = 0;

    // 检查时间段是否有冲突
    virtual bool has_conflict(int spot_id, std::time_t start_time, std::time_t end_time) const = 0;
};

// Token会话数据访问接口
class SessionDAO {
public:
    virtual ~SessionDAO() = default;

    // 创建会话
    virtual void create(const Session& session) = 0;

    // 根据Token查找会话
    virtual std::optional<Session> find_by_token(const std::string& token) const = 0;

    // 过期Token清理
    virtual void cleanup_expired() = 0;

    // 删除会话
    virtual void delete_by_token(const std::string& token) = 0;
};

// 具体实现类
class SQLiteUserDAO : public UserDAO {
public:
    SQLiteUserDAO(Database& db) : db_(db) {}
    
    int create(const User& user) override;
    std::optional<User> find_by_id(int id) const override;
    std::optional<User> find_by_email(const std::string& email) const override;
    bool update(const User& user) override;
    void delete_all(); // 测试用

private:
    Database& db_;
};

class SQLiteParkingSpotDAO : public ParkingSpotDAO {
public:
    SQLiteParkingSpotDAO(Database& db) : db_(db) {}
    
    int create(const ParkingSpot& spot) override;
    std::optional<ParkingSpot> find_by_id(int id) const override;
    std::vector<ParkingSpot> find_by_owner(int user_id) const override;
    std::vector<ParkingSpot> search_available(
        const std::string& city, std::time_t start_time, std::time_t end_time) const override;
    bool update(const ParkingSpot& spot) override;
    bool is_available(int spot_id, std::time_t start_time, std::time_t end_time) const override;
    void delete_all(); // 测试用

private:
    Database& db_;
};

class SQLiteReservationDAO : public ReservationDAO {
public:
    SQLiteReservationDAO(Database& db) : db_(db) {}
    
    int create(const Reservation& reservation) override;
    std::optional<Reservation> find_by_id(int id) const override;
    std::vector<Reservation> find_by_renter(int renter_id) const override;
    std::vector<Reservation> find_by_spot(int spot_id) const override;
    std::vector<Reservation> find_by_owner(int owner_id) const override;
    bool update_status(int id, ReservationStatus status) override;
    bool has_conflict(int spot_id, std::time_t start_time, std::time_t end_time) const override;
    void delete_all(); // 测试用

private:
    Database& db_;
};

class SQLiteSessionDAO : public SessionDAO {
public:
    SQLiteSessionDAO(Database& db) : db_(db) {}
    
    void create(const Session& session) override;
    std::optional<Session> find_by_token(const std::string& token) const override;
    void cleanup_expired() override;
    void delete_by_token(const std::string& token) override;
    void delete_all(); // 测试用

private:
    Database& db_;
};

#endif // PARKING_DAO_H
