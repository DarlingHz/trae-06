#include "ReservationRecordDAO.h"
#include "../util/Logger.h"
#include <stdexcept>
#include <chrono>
#include <ctime>

int ReservationRecordDAO::addReservationRecord(const ReservationRecord& record) {
    std::shared_ptr<Session> session;
    try {
        // 获取数据库连接
        session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for adding reservation record");
            return -1;
        }
        
        // 开始事务
        session->startTransaction();
        
        // 执行插入预约记录的SQL语句
        std::string insert_sql = "INSERT INTO reservation_records (user_id, book_id, reservation_time, status, expire_time, queue_position) VALUES (?, ?, ?, ?, ?, ?)";
        mysqlx::SqlStatement insert_stmt = session->sql(insert_sql);
        insert_stmt.bind(record.getUserId());
        insert_stmt.bind(record.getBookId());
        insert_stmt.bind(record.getReservationDate());
        insert_stmt.bind(record.getStatus());
        insert_stmt.bind(record.getExpireDate());
        insert_stmt.bind(record.getQueuePosition());
        
        mysqlx::SqlResult result = insert_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to insert reservation record into database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return -1;
        }
        
        // 获取插入的预约记录ID
        std::string select_last_insert_id_sql = "SELECT LAST_INSERT_ID()";
        mysqlx::SqlStatement select_last_insert_id_stmt = session->sql(select_last_insert_id_sql);
        mysqlx::SqlResult last_insert_id_result = select_last_insert_id_stmt.execute();
        mysqlx::Row last_insert_id_row = last_insert_id_result.fetchOne();
        int record_id = last_insert_id_row[0].get<int>();
        
        // 更新预约队列位置
        if (!updateReservationQueuePositions(record.getBookId(), record.getStatus())) {
            Logger::error("Failed to update reservation queue positions after adding reservation record");
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return -1;
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Reservation record added successfully, record id: " + std::to_string(record_id));
        DatabaseConnectionPool::releaseConnection(session);
        return record_id;
    } catch (const std::exception& e) {
        Logger::error("Failed to add reservation record: " + std::string(e.what()));
        // Rollback transaction and release connection if session exists
        if (session) {
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
        }
        return -1;
    }
}

bool ReservationRecordDAO::updateReservationRecord(const ReservationRecord& record) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for updating reservation record");
            return false;
        }
        
        // 执行更新预约记录的SQL语句
        std::string update_sql = "UPDATE reservation_records SET user_id = ?, book_id = ?, reservation_time = ?, status = ?, confirm_time = ?, expire_time = ?, queue_position = ? WHERE id = ?";
        mysqlx::SqlStatement update_stmt = session->sql(update_sql);
        update_stmt.bind(record.getUserId());
        update_stmt.bind(record.getBookId());
        update_stmt.bind(record.getReservationDate());
        update_stmt.bind(record.getStatus());
        update_stmt.bind(record.getConfirmedDate());
        update_stmt.bind(record.getExpireDate());
        update_stmt.bind(record.getQueuePosition());
        update_stmt.bind(record.getId());
        
        mysqlx::SqlResult result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to update reservation record in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Reservation record updated successfully, record id: " + std::to_string(record.getId()));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update reservation record: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<ReservationRecord> ReservationRecordDAO::getReservationRecordById(int record_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting reservation record by id");
            return nullptr;
        }
        
        // 执行查询预约记录的SQL语句
        std::string select_sql = "SELECT * FROM reservation_records WHERE id = ?";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(record_id);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("Reservation record not found by id: " + std::to_string(record_id));
            DatabaseConnectionPool::releaseConnection(session);
            return nullptr;
        }
        
        // 从结果集中创建ReservationRecord对象
        mysqlx::Row row = result.fetchOne();
        std::shared_ptr<ReservationRecord> record = createReservationRecordFromResult(row);
        
        DatabaseConnectionPool::releaseConnection(session);
        return record;
    } catch (const std::exception& e) {
        Logger::error("Failed to get reservation record by id: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::shared_ptr<ReservationRecord>> ReservationRecordDAO::getUserReservationRecords(int user_id, const std::string& status, int page, int page_size) {
    std::vector<std::shared_ptr<ReservationRecord>> records;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user reservation records");
            return records;
        }
        
        // 计算分页偏移量
        int offset = (page - 1) * page_size;
        
        // 执行查询用户预约记录的SQL语句
        std::string select_sql = "SELECT * FROM reservation_records WHERE user_id = ?";
        
        // 添加状态过滤条件
        if (!status.empty()) {
            select_sql += " AND status = ?";
        }
        
        // 添加排序、分页条件
        select_sql += " ORDER BY id DESC LIMIT ? OFFSET ?";
        
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        int bind_index = 1;
        select_stmt.bind(bind_index++, user_id);
        
        // 绑定状态参数
        if (!status.empty()) {
            select_stmt.bind(bind_index++, status);
        }
        
        // 绑定分页参数
        select_stmt.bind(bind_index++, page_size);
        select_stmt.bind(bind_index++, offset);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No reservation records found for user id: " + std::to_string(user_id));
            DatabaseConnectionPool::releaseConnection(session);
            return records;
        }
        
        // 从结果集中创建ReservationRecord对象
        for (mysqlx::Row row : result) {
            std::shared_ptr<ReservationRecord> record = createReservationRecordFromResult(row);
            records.push_back(record);
        }
        
        DatabaseConnectionPool::releaseConnection(session);
        return records;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user reservation records: " + std::string(e.what()));
        return records;
    }
}

std::vector<std::shared_ptr<ReservationRecord>> ReservationRecordDAO::getBookReservationRecords(int book_id, const std::string& status) {
    std::vector<std::shared_ptr<ReservationRecord>> records;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting book reservation records");
            return records;
        }
        
        // 执行查询图书预约记录的SQL语句（按预约时间排序）
        std::string select_sql = "SELECT * FROM reservation_records WHERE book_id = ?";
        
        // 添加状态过滤条件
        if (!status.empty()) {
            select_sql += " AND status = ?";
        }
        
        // 添加排序、分页条件
        select_sql += " ORDER BY reservation_time LIMIT 100";
        
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        int bind_index = 1;
        select_stmt.bind(bind_index++, book_id);
        
        // 绑定状态参数
        if (!status.empty()) {
            select_stmt.bind(bind_index++, status);
        }
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No reservation records found for book id: " + std::to_string(book_id));
            DatabaseConnectionPool::releaseConnection(session);
            return records;
        }
        
        // 从结果集中创建ReservationRecord对象
        for (mysqlx::Row row : result) {
            std::shared_ptr<ReservationRecord> record = createReservationRecordFromResult(row);
            records.push_back(record);
        }
        
        DatabaseConnectionPool::releaseConnection(session);
        return records;
    } catch (const std::exception& e) {
        Logger::error("Failed to get book reservation records: " + std::string(e.what()));
        return records;
    }
}

int ReservationRecordDAO::getBookReservationQueueLength(int book_id, const std::string& status) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting book reservation queue length");
            return 0;
        }
        
        // 执行查询图书预约队列长度的SQL语句
        std::string select_sql = "SELECT COUNT(*) FROM reservation_records WHERE book_id = ? AND status = ?";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(book_id);
        select_stmt.bind(status);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No reservation records found for book id: " + std::to_string(book_id));
            DatabaseConnectionPool::releaseConnection(session);
            return 0;
        }
        
        // 获取图书预约队列长度
        mysqlx::Row row = result.fetchOne();
        int queue_length = row[0].get<int>();
        
        DatabaseConnectionPool::releaseConnection(session);
        return queue_length;
    } catch (const std::exception& e) {
        Logger::error("Failed to get book reservation queue length: " + std::string(e.what()));
        return 0;
    }
}

int ReservationRecordDAO::getUserReservationQueuePosition(int user_id, int book_id, const std::string& status) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting user reservation queue position");
            return -1;
        }
        
        // 执行查询用户在图书预约队列中位置的SQL语句
        std::string select_sql = "SELECT queue_position FROM reservation_records WHERE user_id = ? AND book_id = ? AND status = ?";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(user_id);
        select_stmt.bind(book_id);
        select_stmt.bind(status);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No reservation record found for user id: " + std::to_string(user_id) + " and book id: " + std::to_string(book_id));
            DatabaseConnectionPool::releaseConnection(session);
            return -1;
        }
        
        // 获取用户在图书预约队列中的位置
        mysqlx::Row row = result.fetchOne();
        int queue_position = row[0].get<int>();
        
        DatabaseConnectionPool::releaseConnection(session);
        return queue_position;
    } catch (const std::exception& e) {
        Logger::error("Failed to get user reservation queue position: " + std::string(e.what()));
        return -1;
    }
}

bool ReservationRecordDAO::updateReservationQueuePositions(int book_id, const std::string& status) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for updating reservation queue positions");
            return false;
        }
        
        // 开始事务
        session->startTransaction();
        
        // 获取图书预约记录（按预约时间排序）
        std::string select_sql = "SELECT id FROM reservation_records WHERE book_id = ? AND status = ? ORDER BY reservation_time";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(book_id);
        select_stmt.bind(status);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No reservation records found for book id: " + std::to_string(book_id));
            session->commit();
            DatabaseConnectionPool::releaseConnection(session);
            return true;
        }
        
        // 更新预约队列位置
        int queue_position = 1;
        for (mysqlx::Row row : result) {
            int record_id = row[0].get<int>();
            
            std::string update_sql = "UPDATE reservation_records SET queue_position = ? WHERE id = ?";
            mysqlx::SqlStatement update_stmt = session->sql(update_sql);
            update_stmt.bind(queue_position);
            update_stmt.bind(record_id);
            
            update_stmt.execute();
            queue_position++;
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Reservation queue positions updated successfully for book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update reservation queue positions: " + std::string(e.what()));
        return false;
    }
}

bool ReservationRecordDAO::cancelReservationRecord(int record_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for canceling reservation record");
            return false;
        }
        
        // 开始事务
        session->startTransaction();
        
        // 获取预约记录信息
        std::shared_ptr<ReservationRecord> record = getReservationRecordById(record_id);
        if (!record) {
            Logger::error("Failed to get reservation record by id: " + std::to_string(record_id));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 执行更新预约记录状态的SQL语句（取消预约）
        std::string update_sql = "UPDATE reservation_records SET status = ? WHERE id = ?";
        mysqlx::SqlStatement update_stmt = session->sql(update_sql);
        update_stmt.bind("cancelled");
        update_stmt.bind(record_id);
        
        mysqlx::SqlResult result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to cancel reservation record in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 更新预约队列位置
        if (!updateReservationQueuePositions(record->getBookId())) {
            Logger::error("Failed to update reservation queue positions after canceling reservation record");
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Reservation record canceled successfully, record id: " + std::to_string(record_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to cancel reservation record: " + std::string(e.what()));
        return false;
    }
}

bool ReservationRecordDAO::completeReservationRecord(int record_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for completing reservation record");
            return false;
        }
        
        // 开始事务
        session->startTransaction();
        
        // 获取预约记录信息
        std::shared_ptr<ReservationRecord> record = getReservationRecordById(record_id);
        if (!record) {
            Logger::error("Failed to get reservation record by id: " + std::to_string(record_id));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        char now_str[20];
        std::strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", now_tm);
        
        // 执行更新预约记录状态的SQL语句（完成预约）
        std::string update_sql = "UPDATE reservation_records SET status = ?, confirm_time = ? WHERE id = ?";
        mysqlx::SqlStatement update_stmt = session->sql(update_sql);
        update_stmt.bind("completed");
        update_stmt.bind(now_str);
        update_stmt.bind(record_id);
        
        mysqlx::SqlResult result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to complete reservation record in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 更新预约队列位置
        if (!updateReservationQueuePositions(record->getBookId())) {
            Logger::error("Failed to update reservation queue positions after completing reservation record");
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Reservation record completed successfully, record id: " + std::to_string(record_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to complete reservation record: " + std::string(e.what()));
        return false;
    }
}

bool ReservationRecordDAO::expireReservationRecord(int record_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for expiring reservation record");
            return false;
        }
        
        // 开始事务
        session->startTransaction();
        
        // 获取预约记录信息
        std::shared_ptr<ReservationRecord> record = getReservationRecordById(record_id);
        if (!record) {
            Logger::error("Failed to get reservation record by id: " + std::to_string(record_id));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 执行更新预约记录状态的SQL语句（过期预约）
        std::string update_sql = "UPDATE reservation_records SET status = ? WHERE id = ?";
        mysqlx::SqlStatement update_stmt = session->sql(update_sql);
        update_stmt.bind("expired");
        update_stmt.bind(record_id);
        
        mysqlx::SqlResult result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to expire reservation record in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 更新预约队列位置
        if (!updateReservationQueuePositions(record->getBookId())) {
            Logger::error("Failed to update reservation queue positions after expiring reservation record");
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Reservation record expired successfully, record id: " + std::to_string(record_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to expire reservation record: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::shared_ptr<ReservationRecord>> ReservationRecordDAO::scanExpiredReservationRecords() {
    std::vector<std::shared_ptr<ReservationRecord>> expired_records;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for scanning expired reservation records");
            return expired_records;
        }
        
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);
        char now_str[20];
        std::strftime(now_str, sizeof(now_str), "%Y-%m-%d %H:%M:%S", now_tm);
        
        // 开始事务
        session->startTransaction();
        
        // 查询并锁定过期预约记录
        std::string select_sql = "SELECT * FROM reservation_records WHERE status = 'pending' AND expire_time < ? FOR SHARE";
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        select_stmt.bind(now_str);
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No expired reservation records found during scan");
            session->commit();
            DatabaseConnectionPool::releaseConnection(session);
            return expired_records;
        }
        
        // 从结果集中创建ReservationRecord对象
        for (mysqlx::Row row : result) {
            std::shared_ptr<ReservationRecord> record = createReservationRecordFromResult(row);
            expired_records.push_back(record);
            
            // 更新预约记录状态为过期
            std::string update_sql = "UPDATE reservation_records SET status = ? WHERE id = ?";
            mysqlx::SqlStatement update_stmt = session->sql(update_sql);
            update_stmt.bind("expired");
            update_stmt.bind(record->getId());
            
            update_stmt.execute();
            
            // 更新预约队列位置
            updateReservationQueuePositions(record->getBookId());
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Expired reservation records scanned successfully, found " + std::to_string(expired_records.size()) + " expired records");
        DatabaseConnectionPool::releaseConnection(session);
        return expired_records;
    } catch (const std::exception& e) {
        Logger::error("Failed to scan expired reservation records: " + std::string(e.what()));
        return expired_records;
    }
}

int ReservationRecordDAO::getReservationRecordCount(int user_id, int book_id, const std::string& status) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting reservation record count");
            return 0;
        }
        
        // 执行查询预约记录总数的SQL语句
        std::string select_sql = "SELECT COUNT(*) FROM reservation_records";
        std::vector<std::string> conditions;
        std::vector<mysqlx::Value> params;
        
        // 添加用户ID过滤条件
        if (user_id != -1) {
            conditions.push_back("user_id = ?");
            params.push_back(user_id);
        }
        
        // 添加图书ID过滤条件
        if (book_id != -1) {
            conditions.push_back("book_id = ?");
            params.push_back(book_id);
        }
        
        // 添加状态过滤条件
        if (!status.empty()) {
            conditions.push_back("status = ?");
            params.push_back(status);
        }
        
        // 构建完整的SQL语句
        if (!conditions.empty()) {
            select_sql += " WHERE ";
            for (size_t i = 0; i < conditions.size(); ++i) {
                if (i > 0) {
                    select_sql += " AND ";
                }
                select_sql += conditions[i];
            }
        }
        
        mysqlx::SqlStatement select_stmt = session->sql(select_sql);
        
        // 绑定参数
        for (size_t i = 0; i < params.size(); ++i) {
            select_stmt.bind(i + 1, params[i]);
        }
        
        mysqlx::SqlResult result = select_stmt.execute();
        if (result.count() == 0) {
            Logger::debug("No reservation records found for count criteria");
            DatabaseConnectionPool::releaseConnection(session);
            return 0;
        }
        
        // 获取预约记录总数
        mysqlx::Row row = result.fetchOne();
        int record_count = row[0].get<int>();
        
        DatabaseConnectionPool::releaseConnection(session);
        return record_count;
    } catch (const std::exception& e) {
        Logger::error("Failed to get reservation record count: " + std::string(e.what()));
        return 0;
    }
}

std::shared_ptr<ReservationRecord> ReservationRecordDAO::createReservationRecordFromResult(const mysqlx::Row& row) {
    int id = row[0].get<int>();
    int user_id = row[1].get<int>();
    int book_id = row[2].get<int>();
    std::string reservation_date = row[3].get<std::string>();
    std::string status = row[4].get<std::string>();
    std::string confirmed_date = row[5].isNull() ? "" : row[5].get<std::string>();
    std::string expire_date = row[6].get<std::string>();
    int queue_position = row[7].get<int>();
    std::string created_at = row[8].get<std::string>();
    std::string updated_at = row[9].get<std::string>();
    
    return std::make_shared<ReservationRecord>(id, user_id, book_id, reservation_date, status, confirmed_date, expire_date, queue_position, created_at, updated_at);
}