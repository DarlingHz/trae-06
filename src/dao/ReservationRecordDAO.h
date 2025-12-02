#ifndef RESERVATIONRECORDDAO_H
#define RESERVATIONRECORDDAO_H

#include "../model/ReservationRecord.h"
#include "../util/DatabaseConnectionPool.h"
#include <vector>
#include <memory>

class ReservationRecordDAO {
public:
    // 新增预约记录
    static int addReservationRecord(const ReservationRecord& record);
    
    // 更新预约记录
    static bool updateReservationRecord(const ReservationRecord& record);
    
    // 根据ID获取预约记录
    static std::shared_ptr<ReservationRecord> getReservationRecordById(int record_id);
    
    // 获取用户预约记录
    static std::vector<std::shared_ptr<ReservationRecord>> getUserReservationRecords(int user_id, const std::string& status = "", int page = 1, int page_size = 10);
    
    // 获取图书预约记录（按预约时间排序）
    static std::vector<std::shared_ptr<ReservationRecord>> getBookReservationRecords(int book_id, const std::string& status = "");
    
    // 获取图书预约队列长度
    static int getBookReservationQueueLength(int book_id, const std::string& status = "pending");
    
    // 获取用户在图书预约队列中的位置
    static int getUserReservationQueuePosition(int user_id, int book_id, const std::string& status = "pending");
    
    // 更新预约队列位置
    static bool updateReservationQueuePositions(int book_id, const std::string& status = "pending");
    
    // 取消预约记录
    static bool cancelReservationRecord(int record_id);
    
    // 完成预约记录
    static bool completeReservationRecord(int record_id);
    
    // 过期预约记录
    static bool expireReservationRecord(int record_id);
    
    // 扫描过期预约记录
    static std::vector<std::shared_ptr<ReservationRecord>> scanExpiredReservationRecords();
    
    // 获取预约记录总数
    static int getReservationRecordCount(int user_id = -1, int book_id = -1, const std::string& status = "");
    
private:
    // 从数据库结果集中创建ReservationRecord对象
    static std::shared_ptr<ReservationRecord> createReservationRecordFromResult(const mysqlx::Row& row);
};

#endif // RESERVATIONRECORDDAO_H