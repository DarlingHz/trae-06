#include "BorrowService.h"
#include "../dao/BorrowRecordDAO.h"
#include "../service/BookService.h"
#include "../util/Logger.h"
#include "../util/Config.h"
#include <stdexcept>
#include <chrono>
#include <ctime>

int BorrowService::borrowBook(int user_id, int book_id) {
    try {
        // 检查用户是否可以借阅图书
        if (!checkUserCanBorrow(user_id, book_id)) {
            Logger::error("User cannot borrow book: user_id=" + std::to_string(user_id) + ", book_id=" + std::to_string(book_id));
            return -1;
        }
        
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 创建BookService对象
        BookService book_service;
        
        // 开始事务
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for borrowing book");
            return -1;
        }
        
        session->startTransaction();
        
        // 增加图书在借数量
        if (!book_service.incrementBorrowCount(book_id)) {
            Logger::error("Failed to increment book borrow count: book_id=" + std::to_string(book_id));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return -1;
        }
        
        // 获取借阅期限
        int borrow_days = Config::getLibraryBorrowPeriodDays();
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        char borrow_time_str[20];
        std::strftime(borrow_time_str, sizeof(borrow_time_str), "%Y-%m-%d %H:%M:%S", now_tm);
        
        // 计算应还时间
        auto due_time = now + std::chrono::hours(borrow_days * 24);
        std::time_t due_time_time = std::chrono::system_clock::to_time_t(due_time);
        std::tm* due_time_tm = std::localtime(&due_time_time);
        char due_time_str[20];
        std::strftime(due_time_str, sizeof(due_time_str), "%Y-%m-%d %H:%M:%S", due_time_tm);
        
        // 创建借阅记录
        BorrowRecord borrow_record;
        borrow_record.setUserId(user_id);
        borrow_record.setBookId(book_id);
        borrow_record.setBorrowDate(borrow_time_str);
        borrow_record.setDueDate(due_time_str);
        borrow_record.setStatus("borrowed");
        
        // 新增借阅记录
        int borrow_id = borrow_dao.addBorrowRecord(borrow_record);
        if (borrow_id == -1) {
            Logger::error("Failed to add borrow record: user_id=" + std::to_string(user_id) + ", book_id=" + std::to_string(book_id));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return -1;
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Book borrowed successfully: user_id=" + std::to_string(user_id) + ", book_id=" + std::to_string(book_id) + ", borrow_id=" + std::to_string(borrow_id));
        DatabaseConnectionPool::releaseConnection(session);
        return borrow_id;
    } catch (const std::exception& e) {
        Logger::error("Failed to borrow book: " + std::string(e.what()));
        return -1;
    }
}

bool BorrowService::returnBook(int borrow_id) {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 创建BookService对象
        BookService book_service;
        
        // 获取借阅记录
        std::shared_ptr<BorrowRecord> borrow_record = borrow_dao.getBorrowRecordById(borrow_id);
        if (!borrow_record) {
            Logger::error("Borrow record not found: borrow_id=" + std::to_string(borrow_id));
            return false;
        }
        
        // 检查借阅记录是否已经归还
        if (borrow_record->getStatus() != "borrowed") {
            Logger::error("Borrow record is not in borrowed status: borrow_id=" + std::to_string(borrow_id));
            return false;
        }
        
        // 开始事务
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for returning book");
            return false;
        }
        
        session->startTransaction();
        
        // 减少图书在借数量
        if (!book_service.decrementBorrowCount(borrow_record->getBookId())) {
            Logger::error("Failed to decrement book borrow count: book_id=" + std::to_string(borrow_record->getBookId()));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        char return_time_str[20];
        std::strftime(return_time_str, sizeof(return_time_str), "%Y-%m-%d %H:%M:%S", now_tm);
        
        // 检查是否逾期
        std::tm due_time_tm = {};
        std::istringstream due_time_ss(borrow_record->getDueDate());
        due_time_ss >> std::get_time(&due_time_tm, "%Y-%m-%d %H:%M:%S");
        std::time_t due_time_time = std::mktime(&due_time_tm);
        
        std::string status = "returned";
        if (now_time > due_time_time) {
            status = "overdue_returned";
        }
        
        // 更新借阅记录
        BorrowRecord updated_record = *borrow_record;
        updated_record.setReturnDate(return_time_str);
        updated_record.setStatus(status);
        
        if (!borrow_dao.updateBorrowRecord(updated_record)) {
            Logger::error("Failed to update borrow record: borrow_id=" + std::to_string(borrow_id));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Book returned successfully: borrow_id=" + std::to_string(borrow_id) + ", status=" + status);
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to return book: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<BorrowRecord> BorrowService::getBorrowRecordById(int borrow_id) {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 获取借阅记录
        std::shared_ptr<BorrowRecord> borrow_record = borrow_dao.getBorrowRecordById(borrow_id);
        
        if (borrow_record) {
            Logger::info("Borrow record retrieved successfully: borrow_id=" + std::to_string(borrow_id));
        } else {
            Logger::error("Failed to retrieve borrow record: borrow_id=" + std::to_string(borrow_id));
        }
        
        return borrow_record;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve borrow record: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowService::getUserBorrowRecords(int user_id, const std::string& status, int page, int page_size) {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 获取用户借阅记录
        std::vector<std::shared_ptr<BorrowRecord>> borrow_records = borrow_dao.getUserBorrowRecords(user_id, status, page, page_size);
        
        Logger::info("User borrow records retrieved successfully: user_id=" + std::to_string(user_id) + ", status=" + status + ", page=" + std::to_string(page) + ", page_size=" + std::to_string(page_size));
        return borrow_records;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve user borrow records: " + std::string(e.what()));
        return std::vector<std::shared_ptr<BorrowRecord>>();
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowService::getBookBorrowRecords(int book_id, const std::string& status) {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 获取图书借阅记录
        std::vector<std::shared_ptr<BorrowRecord>> borrow_records = borrow_dao.getBookBorrowRecords(book_id, status);
        
        Logger::info("Book borrow records retrieved successfully: book_id=" + std::to_string(book_id) + ", status=" + status);
        return borrow_records;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve book borrow records: " + std::string(e.what()));
        return std::vector<std::shared_ptr<BorrowRecord>>();
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowService::getOverdueBorrowRecords(int page, int page_size) {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 获取逾期借阅记录
        std::vector<std::shared_ptr<BorrowRecord>> borrow_records = borrow_dao.getOverdueBorrowRecords(page, page_size);
        
        Logger::info("Overdue borrow records retrieved successfully: page=" + std::to_string(page) + ", page_size=" + std::to_string(page_size));
        return borrow_records;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve overdue borrow records: " + std::string(e.what()));
        return std::vector<std::shared_ptr<BorrowRecord>>();
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowService::scanOverdueBorrowRecords() {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 扫描逾期借阅记录
        std::vector<std::shared_ptr<BorrowRecord>> overdue_records = borrow_dao.scanOverdueBorrowRecords();
        
        Logger::info("Overdue borrow records scanned successfully: found " + std::to_string(overdue_records.size()) + " overdue records");
        return overdue_records;
    } catch (const std::exception& e) {
        Logger::error("Failed to scan overdue borrow records: " + std::string(e.what()));
        return std::vector<std::shared_ptr<BorrowRecord>>();
    }
}

int BorrowService::getUserCurrentBorrowCount(int user_id) {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 获取用户当前借阅数量
        int count = borrow_dao.getUserCurrentBorrowCount(user_id);
        
        Logger::info("User current borrow count retrieved successfully: user_id=" + std::to_string(user_id) + ", count=" + std::to_string(count));
        return count;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve user current borrow count: " + std::string(e.what()));
        return -1;
    }
}

int BorrowService::getBorrowRecordCount(int user_id, int book_id, const std::string& status) {
    try {
        // 创建BorrowRecordDAO对象
        BorrowRecordDAO borrow_dao;
        
        // 获取借阅记录总数
        int count = borrow_dao.getBorrowRecordCount(user_id, book_id, status);
        
        Logger::info("Borrow record count retrieved successfully: user_id=" + std::to_string(user_id) + ", book_id=" + std::to_string(book_id) + ", status=" + status);
        return count;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve borrow record count: " + std::string(e.what()));
        return 0;
    }
}

bool BorrowService::checkUserCanBorrow(int user_id, int book_id) {
    try {
        // 创建BookService对象
        BookService book_service;
        
        // 检查图书是否可借
        if (!book_service.checkBookAvailable(book_id)) {
            Logger::error("Book not available: book_id=" + std::to_string(book_id));
            return false;
        }
        
        // 获取用户当前借阅数量
        int current_borrow_count = getUserCurrentBorrowCount(user_id);
        if (current_borrow_count == -1) {
            Logger::error("Failed to get user current borrow count: user_id=" + std::to_string(user_id));
            return false;
        }
        
        // 获取借阅上限
        int max_borrow_count = Config::getLibraryMaxBorrowBooks();
        
        // 检查是否超过借阅上限
        if (current_borrow_count >= max_borrow_count) {
            Logger::error("User exceeded maximum borrow count: user_id=" + std::to_string(user_id) + ", current_count=" + std::to_string(current_borrow_count) + ", max_count=" + std::to_string(max_borrow_count));
            return false;
        }
        
        // 检查用户是否有逾期未还的图书
        int overdue_count = getBorrowRecordCount(user_id, -1, "overdue");
        if (overdue_count > 0) {
            Logger::error("User has overdue books: user_id=" + std::to_string(user_id) + ", overdue_count=" + std::to_string(overdue_count));
            return false;
        }
        
        Logger::info("User can borrow book: user_id=" + std::to_string(user_id) + ", book_id=" + std::to_string(book_id));
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to check user can borrow: " + std::string(e.what()));
        return false;
    }
}