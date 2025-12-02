#include "BorrowRecordDAO.h"
#include "../util/Logger.h"
#include <stdexcept>
#include <chrono>
#include <ctime>

bool BorrowRecordDAO::addBorrowRecord(const BorrowRecord& record) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for adding borrow record");
            return false;
        }
        
        // 执行插入借阅记录的SQL语句
        std::string insert_sql = "INSERT INTO borrow_records (user_id, book_id, borrow_date, due_date, status) VALUES (?, ?, ?, ?, ?)";
        mysqlx::SqlStatement insert_stmt = session->sql(insert_sql);
        insert_stmt.bind(1, record.getUserId());
        insert_stmt.bind(2, record.getBookId());
        insert_stmt.bind(3, record.getBorrowDate());
        insert_stmt.bind(4, record.getDueDate());
        insert_stmt.bind(5, record.getStatus());
        
        mysqlx::SqlResult result = insert_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to insert borrow record into database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Borrow record added successfully");
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to add borrow record: " + std::string(e.what()));
        return false;
    }
}

bool BorrowRecordDAO::updateBorrowRecord(const BorrowRecord& record) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for updating borrow record");
            return false;
        }
        
        // 执行更新借阅记录的SQL语句
        std::string update_sql = "UPDATE borrow_records SET user_id = ?, book_id = ?, borrow_date = ?, due_date = ?, return_date = ?, status = ? WHERE id = ?";
        mysqlx::SqlStatement update_stmt = session->sql(update_sql);
        update_stmt.bind(1, record.getUserId());
        update_stmt.bind(2, record.getBookId());
        update_stmt.bind(3, record.getBorrowDate());
        update_stmt.bind(4, record.getDueDate());
        update_stmt.bind(5, record.getReturnDate());
        update_stmt.bind(6, record.getStatus());
        update_stmt.bind(7, record.getId());
        
        mysqlx::SqlResult result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to update borrow record in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Borrow record updated successfully, record id: " + std::to_string(record.getId()));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update borrow record: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<BorrowRecord> BorrowRecordDAO::getBorrowRecordById(int record_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting borrow record by id");
            return nullptr;
        }
        
        // 执行查询借阅记录的SQL语句
        std::string select_sql = "SELECT * FROM borrow_records WHERE id = ?";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(1, record_id);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("Borrow record not found by id: " + std::to_string(record_id));
            DatabaseConnectionPool::releaseConnection(session);
            return nullptr;
        }
        
        // 从结果集中创建BorrowRecord对象
        mysqlx::Row row = result.fetchOne();
        std::shared_ptr<BorrowRecord> record = createBorrowRecordFromResult(row);
        
        DatabaseConnectionPool::releaseConnection(session);
        return record;
    } catch (const std::exception& e) {
        Logger::error("Failed to get borrow record by id: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowRecordDAO::getUserBorrowRecords(int user_id, const std::string& status, int page, int page_size) {
    std::vector<std::shared_ptr<BorrowRecord>> records;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user borrow records");
            return records;
        }
        
        // 计算分页偏移量
        int offset = (page - 1) * page_size;
        
        // 执行查询用户借阅记录的SQL语句
        std::string select_sql = "SELECT * FROM borrow_records WHERE user_id = ?";
        std::vector<mysqlx::Value> values;
        
        values.push_back(user_id);
        
        // 添加状态过滤条件
        if (!status.empty()) {
            select_sql += " AND status = ?";
            values.push_back(status);
        }
        
        // 添加排序和分页条件
        select_sql += " ORDER BY id DESC LIMIT ? OFFSET ?";
        values.push_back(page_size);
        values.push_back(offset);
        
        // 执行查询语句
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        for (size_t i = 0; i < values.size(); i++) {
            select_stmt.bind(i + 1, values[i]);
        }
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No borrow records found for user id: " + std::to_string(user_id));
            DatabaseConnectionPool::releaseConnection(session);
            return records;
        }
        
        // 从结果集中创建BorrowRecord对象
        for (mysqlx::Row row : result) {
            std::shared_ptr<BorrowRecord> record = createBorrowRecordFromResult(row);
            records.push_back(record);
        }
        
        DatabaseConnectionPool::releaseConnection(session);
        return records;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user borrow records: " + std::string(e.what()));
        return records;
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowRecordDAO::getBookBorrowRecords(int book_id, const std::string& status, int page, int page_size) {
    std::vector<std::shared_ptr<BorrowRecord>> records;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting book borrow records");
            return records;
        }
        
        // 计算分页偏移量
        int offset = (page - 1) * page_size;
        
        // 执行查询图书借阅记录的SQL语句
        std::string select_sql = "SELECT * FROM borrow_records WHERE book_id = ?";
        std::vector<mysqlx::Value> values;
        
        values.push_back(book_id);
        
        // 添加状态过滤条件
        if (!status.empty()) {
            select_sql += " AND status = ?";
            values.push_back(status);
        }
        
        // 添加排序和分页条件
        select_sql += " ORDER BY id DESC LIMIT ? OFFSET ?";
        values.push_back(page_size);
        values.push_back(offset);
        
        // 执行查询语句
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        for (size_t i = 0; i < values.size(); i++) {
            select_stmt.bind(i + 1, values[i]);
        }
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No borrow records found for book id: " + std::to_string(book_id));
            DatabaseConnectionPool::releaseConnection(session);
            return records;
        }
        
        // 从结果集中创建BorrowRecord对象
        for (mysqlx::Row row : result) {
            std::shared_ptr<BorrowRecord> record = createBorrowRecordFromResult(row);
            records.push_back(record);
        }
        
        DatabaseConnectionPool::releaseConnection(session);
        return records;
    } catch (const std::exception& e) {
        Logger::error("Failed to get book borrow records: " + std::string(e.what()));
        return records;
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowRecordDAO::getOverdueBorrowRecords(int page, int page_size) {
    std::vector<std::shared_ptr<BorrowRecord>> records;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting overdue borrow records");
            return records;
        }
        
        // 计算分页偏移量
        int offset = (page - 1) * page_size;
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        char now_str[20];
        std::strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", now_tm);
        
        // 执行查询逾期借阅记录的SQL语句
        std::string select_sql = "SELECT * FROM borrow_records WHERE status = 'borrowed' AND due_time < ? ORDER BY due_time LIMIT ? OFFSET ?";
        std::vector<mysqlx::Value> values;
        
        values.push_back(now_str);
        values.push_back(page_size);
        values.push_back(offset);
        
        // 执行查询语句
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        for (size_t i = 0; i < values.size(); i++) {
            select_stmt.bind(i + 1, values[i]);
        }
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No overdue borrow records found");
            DatabaseConnectionPool::releaseConnection(session);
            return records;
        }
        
        // 从结果集中创建BorrowRecord对象
        for (mysqlx::Row row : result) {
            std::shared_ptr<BorrowRecord> record = createBorrowRecordFromResult(row);
            records.push_back(record);
        }
        
        DatabaseConnectionPool::releaseConnection(session);
        return records;
    } catch (const std::exception& e) {
        Logger::error("Failed to get overdue borrow records: " + std::string(e.what()));
        return records;
    }
}

std::vector<std::shared_ptr<BorrowRecord>> BorrowRecordDAO::scanOverdueBorrowRecords() {
    std::vector<std::shared_ptr<BorrowRecord>> overdue_records;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for scanning overdue borrow records");
            return overdue_records;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        char now_str[20];
        std::strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", now_tm);
        
        // 开始事务
        session->startTransaction();
        
        // 查询并锁定逾期借阅记录
        std::string select_sql = "SELECT * FROM borrow_records WHERE status = 'borrowed' AND due_time < ? FOR SHARE";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(1, now_str);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No overdue borrow records found during scan");
            session->commit();
            DatabaseConnectionPool::releaseConnection(session);
            return overdue_records;
        }
        
        // 从结果集中创建BorrowRecord对象
        for (mysqlx::Row row : result) {
            std::shared_ptr<BorrowRecord> record = createBorrowRecordFromResult(row);
            overdue_records.push_back(record);
            
            // 更新借阅记录状态为逾期
            std::string update_sql = "UPDATE borrow_records SET status = 'overdue' WHERE id = ?";
            mysqlx::SqlStatement update_stmt = session->sql(update_sql);
            update_stmt.bind(1, record->getId());
            update_stmt.execute();
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Overdue borrow records scanned successfully, found " + std::to_string(overdue_records.size()) + " overdue records");
        DatabaseConnectionPool::releaseConnection(session);
        return overdue_records;
    } catch (const std::exception& e) {
        Logger::error("Failed to scan overdue borrow records: " + std::string(e.what()));
        return overdue_records;
    }
}

int BorrowRecordDAO::getUserCurrentBorrowCount(int user_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user current borrow count");
            return 0;
        }
        
        // 执行查询用户当前借阅数量的SQL语句
        std::string select_sql = "SELECT COUNT(*) FROM borrow_records WHERE user_id = ? AND status IN ('borrowed', 'overdue')";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(1, user_id);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No borrow records found for user id: " + std::to_string(user_id));
            DatabaseConnectionPool::releaseConnection(session);
            return 0;
        }
        
        // 获取用户当前借阅数量
        mysqlx::Row row = result.fetchOne();
        int borrow_count = row[0].get<int>();
        
        DatabaseConnectionPool::releaseConnection(session);
        return borrow_count;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user current borrow count: " + std::string(e.what()));
        return 0;
    }
}

int BorrowRecordDAO::getBorrowRecordCount(int user_id, int book_id, const std::string& status) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting borrow record count");
            return 0;
        }
        
        // 执行查询借阅记录总数的SQL语句
        std::string select_sql = "SELECT COUNT(*) FROM borrow_records";
        std::vector<mysqlx::Value> values;
        
        // 添加用户ID过滤条件
        if (user_id != -1) {
            select_sql += " WHERE user_id = ?";
            values.push_back(user_id);
        }
        
        // 添加图书ID过滤条件
        if (book_id != -1) {
            if (user_id != -1) {
                select_sql += " AND book_id = ?";
            } else {
                select_sql += " WHERE book_id = ?";
            }
            values.push_back(book_id);
        }
        
        // 添加状态过滤条件
        if (!status.empty()) {
            if (user_id != -1 || book_id != -1) {
                select_sql += " AND status = ?";
            } else {
                select_sql += " WHERE status = ?";
            }
            values.push_back(status);
        }
        
        // 执行查询语句
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        for (size_t i = 0; i < values.size(); i++) {
            select_stmt.bind(i + 1, values[i]);
        }
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No borrow records found for count criteria");
            DatabaseConnectionPool::releaseConnection(session);
            return 0;
        }
        
        // 获取借阅记录总数
        mysqlx::Row row = result.fetchOne();
        int record_count = row[0].get<int>();
        
        DatabaseConnectionPool::releaseConnection(session);
        return record_count;
    } catch (const std::exception& e) {
        Logger::error("Failed to get borrow record count: " + std::string(e.what()));
        return 0;
    }
}

std::shared_ptr<BorrowRecord> BorrowRecordDAO::createBorrowRecordFromResult(const mysqlx::Row& row) {
    int id = row[0].get<int>();
    int user_id = row[1].get<int>();
    int book_id = row[2].get<int>();
    std::string borrow_date = row[3].get<std::string>();
    std::string due_date = row[4].get<std::string>();
    std::string return_date = row[5].isNull() ? "" : row[5].get<std::string>();
    std::string status = row[6].get<std::string>();
    std::string created_at = row[7].get<std::string>();
    std::string updated_at = row[8].get<std::string>();
    
    return std::make_shared<BorrowRecord>(id, user_id, book_id, borrow_date, due_date, return_date, status, created_at, updated_at);
}