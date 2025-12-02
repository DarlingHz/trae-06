#ifndef BORROWRECORDDAO_H
#define BORROWRECORDDAO_H

#include "../model/BorrowRecord.h"
#include "../util/DatabaseConnectionPool.h"
#include <vector>
#include <memory>

class BorrowRecordDAO {
public:
    // 新增借阅记录
    static bool addBorrowRecord(const BorrowRecord& record);
    
    // 更新借阅记录
    static bool updateBorrowRecord(const BorrowRecord& record);
    
    // 根据ID获取借阅记录
    static std::shared_ptr<BorrowRecord> getBorrowRecordById(int record_id);
    
    // 获取用户借阅记录
    static std::vector<std::shared_ptr<BorrowRecord>> getUserBorrowRecords(int user_id, const std::string& status = "", int page = 1, int page_size = 10);
    
    // 获取图书借阅记录
    static std::vector<std::shared_ptr<BorrowRecord>> getBookBorrowRecords(int book_id, const std::string& status = "", int page = 1, int page_size = 10);
    
    // 获取逾期借阅记录
    static std::vector<std::shared_ptr<BorrowRecord>> getOverdueBorrowRecords(int page = 1, int page_size = 10);
    
    // 扫描逾期借阅记录
    static std::vector<std::shared_ptr<BorrowRecord>> scanOverdueBorrowRecords();
    
    // 获取用户当前借阅数量
    static int getUserCurrentBorrowCount(int user_id);
    
    // 获取借阅记录总数
    static int getBorrowRecordCount(int user_id = -1, int book_id = -1, const std::string& status = "");
    
private:
    // 从数据库结果集中创建BorrowRecord对象
    static std::shared_ptr<BorrowRecord> createBorrowRecordFromResult(const Row& row);
};

#endif // BORROWRECORDDAO_H