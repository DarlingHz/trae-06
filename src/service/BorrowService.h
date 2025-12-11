#ifndef BORROW_SERVICE_H
#define BORROW_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include "../model/BorrowRecord.h"

class BorrowService {
public:
    /**
     * 借阅图书
     * @param user_id 用户ID
     * @param book_id 图书ID
     * @return 借阅成功返回借阅记录ID，否则返回-1
     */
    int borrowBook(int user_id, int book_id);
    
    /**
     * 归还图书
     * @param borrow_id 借阅记录ID
     * @return 归还成功返回true，否则返回false
     */
    bool returnBook(int borrow_id);
    
    /**
     * 获取借阅记录
     * @param borrow_id 借阅记录ID
     * @return 获取成功返回借阅记录对象，否则返回nullptr
     */
    std::shared_ptr<BorrowRecord> getBorrowRecordById(int borrow_id);
    
    /**
     * 获取用户借阅记录
     * @param user_id 用户ID
     * @param status 借阅状态
     * @param page 页码
     * @param page_size 每页大小
     * @return 获取成功返回借阅记录列表，否则返回空列表
     */
    std::vector<std::shared_ptr<BorrowRecord>> getUserBorrowRecords(int user_id, const std::string& status, int page, int page_size);
    
    /**
     * 获取图书借阅记录
     * @param book_id 图书ID
     * @param status 借阅状态
     * @return 获取成功返回借阅记录列表，否则返回空列表
     */
    std::vector<std::shared_ptr<BorrowRecord>> getBookBorrowRecords(int book_id, const std::string& status);
    
    /**
     * 获取逾期借阅记录
     * @param page 页码
     * @param page_size 每页大小
     * @return 获取成功返回逾期借阅记录列表，否则返回空列表
     */
    std::vector<std::shared_ptr<BorrowRecord>> getOverdueBorrowRecords(int page, int page_size);
    
    /**
     * 扫描逾期借阅记录
     * @return 扫描成功返回逾期借阅记录列表，否则返回空列表
     */
    std::vector<std::shared_ptr<BorrowRecord>> scanOverdueBorrowRecords();
    
    /**
     * 获取用户当前借阅数量
     * @param user_id 用户ID
     * @return 获取成功返回用户当前借阅数量，否则返回-1
     */
    int getUserCurrentBorrowCount(int user_id);
    
    /**
     * 获取借阅记录总数
     * @param user_id 用户ID
     * @param book_id 图书ID
     * @param status 借阅状态
     * @return 获取成功返回借阅记录总数，否则返回0
     */
    int getBorrowRecordCount(int user_id, int book_id, const std::string& status);
    
    /**
     * 检查用户是否可以借阅图书
     * @param user_id 用户ID
     * @param book_id 图书ID
     * @return 可以借阅返回true，否则返回false
     */
    bool checkUserCanBorrow(int user_id, int book_id);
};

#endif // BORROW_SERVICE_H