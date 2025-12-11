#ifndef RESERVATION_SERVICE_H
#define RESERVATION_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include "../model/ReservationRecord.h"

class ReservationService {
public:
    /**
     * 预约图书
     * @param user_id 用户ID
     * @param book_id 图书ID
     * @return 预约成功返回预约记录ID，否则返回-1
     */
    int reserveBook(int user_id, int book_id);
    
    /**
     * 取消预约
     * @param reservation_id 预约记录ID
     * @return 取消成功返回true，否则返回false
     */
    bool cancelReservation(int reservation_id);
    
    /**
     * 完成预约
     * @param reservation_id 预约记录ID
     * @return 完成成功返回true，否则返回false
     */
    bool completeReservation(int reservation_id);
    
    /**
     * 过期预约
     * @param reservation_id 预约记录ID
     * @return 过期成功返回true，否则返回false
     */
    bool expireReservation(int reservation_id);
    
    /**
     * 获取预约记录
     * @param reservation_id 预约记录ID
     * @return 获取成功返回预约记录对象，否则返回nullptr
     */
    std::shared_ptr<ReservationRecord> getReservationRecordById(int reservation_id);
    
    /**
     * 获取用户预约记录
     * @param user_id 用户ID
     * @param status 预约状态
     * @param page 页码
     * @param page_size 每页大小
     * @return 获取成功返回预约记录列表，否则返回空列表
     */
    std::vector<std::shared_ptr<ReservationRecord>> getUserReservationRecords(int user_id, const std::string& status, int page, int page_size);
    
    /**
     * 获取图书预约记录
     * @param book_id 图书ID
     * @param status 预约状态
     * @return 获取成功返回预约记录列表，否则返回空列表
     */
    std::vector<std::shared_ptr<ReservationRecord>> getBookReservationRecords(int book_id, const std::string& status);
    
    /**
     * 获取图书预约队列长度
     * @param book_id 图书ID
     * @param status 预约状态
     * @return 获取成功返回图书预约队列长度，否则返回0
     */
    int getBookReservationQueueLength(int book_id, const std::string& status);
    
    /**
     * 获取用户预约队列位置
     * @param user_id 用户ID
     * @param book_id 图书ID
     * @param status 预约状态
     * @return 获取成功返回用户预约队列位置，否则返回-1
     */
    int getUserReservationQueuePosition(int user_id, int book_id, const std::string& status);
    
    /**
     * 扫描过期预约记录
     * @return 扫描成功返回过期预约记录列表，否则返回空列表
     */
    std::vector<std::shared_ptr<ReservationRecord>> scanExpiredReservationRecords();
    
    /**
     * 检查用户是否可以预约图书
     * @param user_id 用户ID
     * @param book_id 图书ID
     * @return 可以预约返回true，否则返回false
     */
    bool checkUserCanReserve(int user_id, int book_id);
    
    /**
     * 获取预约记录总数
     * @param user_id 用户ID
     * @param book_id 图书ID
     * @param status 预约状态
     * @return 获取成功返回预约记录总数，否则返回0
     */
    int getReservationRecordCount(int user_id = -1, int book_id = -1, const std::string& status = "");
    
    /**
     * 处理图书归还后的预约队列
     * @param book_id 图书ID
     * @return 处理成功返回true，否则返回false
     */
    bool processReservationQueue(int book_id);
};

#endif // RESERVATION_SERVICE_H