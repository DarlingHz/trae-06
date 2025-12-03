#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <map>
#include "../models/repair_order.h"
#include "../models/repair_status_history.h"
#include "../db/db_pool.h"
#include "../utils/logger.h"
#include "../utils/date_utils.h"

class RepairOrderRepository {
public:
    std::shared_ptr<RepairOrder> create(const RepairOrder& repairOrder) {
        if (!repairOrder.isValid()) {
            throw std::runtime_error("Invalid RepairOrder data");
        }

        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "INSERT INTO repair_orders (device_id, user_id, service_center_id, status, "
                          "problem_description, expected_finish_date, created_at, updated_at) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        std::string statusStr = RepairOrder::statusToString(repairOrder.status);
        std::string now = DateUtils::getToday();

        MYSQL_BIND bind[8];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &repairOrder.deviceId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &repairOrder.userId;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &repairOrder.serviceCenterId;

        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)statusStr.c_str();
        bind[3].buffer_length = statusStr.length();

        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)repairOrder.problemDescription.c_str();
        bind[4].buffer_length = repairOrder.problemDescription.length();

        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = repairOrder.expectedFinishDate.empty() ? nullptr : (char*)repairOrder.expectedFinishDate.c_str();
        bind[5].buffer_length = repairOrder.expectedFinishDate.length();
        bind[5].is_null = repairOrder.expectedFinishDate.empty() ? &is_null : nullptr;

        bind[6].buffer_type = MYSQL_TYPE_STRING;
        bind[6].buffer = (char*)now.c_str();
        bind[6].buffer_length = now.length();

        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = (char*)now.c_str();
        bind[7].buffer_length = now.length();

        bool is_null = true;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        auto repairOrderPtr = std::make_shared<RepairOrder>(repairOrder);
        repairOrderPtr->id = mysql_stmt_insert_id(stmt.get());
        repairOrderPtr->createdAt = now;
        repairOrderPtr->updatedAt = now;
        return repairOrderPtr;
    }

    std::shared_ptr<RepairOrder> findById(int id) {
        if (id <= 0) {
            return nullptr;
        }

        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "SELECT id, device_id, user_id, service_center_id, status, "
                          "problem_description, expected_finish_date, created_at, updated_at "
                          "FROM repair_orders WHERE id = ?";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        RepairOrder repairOrder;
        MYSQL_BIND resultBind[9];
        memset(resultBind, 0, sizeof(resultBind));

        char status[50] = {0};
        char problemDesc[1000] = {0};
        char expectedFinish[100] = {0};
        char createdAt[100] = {0};
        char updatedAt[100] = {0};
        unsigned long statusLength, problemDescLength, expectedFinishLength, createdAtLength, updatedAtLength;
        bool expectedFinishIsNull = false;

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &repairOrder.id;

        resultBind[1].buffer_type = MYSQL_TYPE_LONG;
        resultBind[1].buffer = &repairOrder.deviceId;

        resultBind[2].buffer_type = MYSQL_TYPE_LONG;
        resultBind[2].buffer = &repairOrder.userId;

        resultBind[3].buffer_type = MYSQL_TYPE_LONG;
        resultBind[3].buffer = &repairOrder.serviceCenterId;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = status;
        resultBind[4].buffer_length = sizeof(status);
        resultBind[4].length = &statusLength;

        resultBind[5].buffer_type = MYSQL_TYPE_STRING;
        resultBind[5].buffer = problemDesc;
        resultBind[5].buffer_length = sizeof(problemDesc);
        resultBind[5].length = &problemDescLength;

        resultBind[6].buffer_type = MYSQL_TYPE_STRING;
        resultBind[6].buffer = expectedFinish;
        resultBind[6].buffer_length = sizeof(expectedFinish);
        resultBind[6].length = &expectedFinishLength;
        resultBind[6].is_null = &expectedFinishIsNull;

        resultBind[7].buffer_type = MYSQL_TYPE_STRING;
        resultBind[7].buffer = createdAt;
        resultBind[7].buffer_length = sizeof(createdAt);
        resultBind[7].length = &createdAtLength;

        resultBind[8].buffer_type = MYSQL_TYPE_STRING;
        resultBind[8].buffer = updatedAt;
        resultBind[8].buffer_length = sizeof(updatedAt);
        resultBind[8].length = &updatedAtLength;

        if (mysql_stmt_bind_result(stmt.get(), resultBind) != 0) {
            throw std::runtime_error(std::string("Failed to bind result: ") + mysql_error(conn));
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            repairOrder.status = RepairOrder::statusFromString(std::string(status, statusLength));
            repairOrder.problemDescription = std::string(problemDesc, problemDescLength);
            if (!expectedFinishIsNull) {
                repairOrder.expectedFinishDate = std::string(expectedFinish, expectedFinishLength);
            }
            repairOrder.createdAt = std::string(createdAt, createdAtLength);
            repairOrder.updatedAt = std::string(updatedAt, updatedAtLength);
            return std::make_shared<RepairOrder>(repairOrder);
        }

        return nullptr;
    }

    std::vector<std::shared_ptr<RepairOrder>> findByFilters(int userId = 0, int serviceCenterId = 0,
        const std::string& status = "", const std::string& city = "", const std::string& startDate = "",
        const std::string& endDate = "", const std::string& sortBy = "created_at", bool ascending = true,
        int page = 1, int pageSize = 10) {
        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "SELECT ro.id, ro.device_id, ro.user_id, ro.service_center_id, ro.status, "
                          "ro.problem_description, ro.expected_finish_date, ro.created_at, ro.updated_at "
                          "FROM repair_orders ro "
                          "LEFT JOIN service_centers sc ON ro.service_center_id = sc.id ";

        std::string whereClause = "WHERE 1=1";
        std::vector<std::string> conditions;

        if (userId > 0) {
            conditions.push_back("ro.user_id = ?");
        }
        if (serviceCenterId > 0) {
            conditions.push_back("ro.service_center_id = ?");
        }
        if (!status.empty()) {
            conditions.push_back("ro.status = ?");
        }
        if (!city.empty()) {
            conditions.push_back("sc.city LIKE ?");
        }
        if (!startDate.empty()) {
            conditions.push_back("ro.created_at >= ?");
        }
        if (!endDate.empty()) {
            conditions.push_back("ro.created_at <= ?");
        }

        for (const auto& cond : conditions) {
            whereClause += " AND " + cond;
        }

        std::string sortClause = " ORDER BY ro." + sortBy;
        if (!ascending) {
            sortClause += " DESC";
        }

        int offset = (page - 1) * pageSize;
        std::string limitClause = " LIMIT ? OFFSET ?";

        sql += whereClause + sortClause + limitClause;

        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        MYSQL_BIND bind[10];
        memset(bind, 0, sizeof(bind));
        int paramIndex = 0;

        if (userId > 0) {
            bind[paramIndex].buffer_type = MYSQL_TYPE_LONG;
            bind[paramIndex].buffer = &userId;
            paramIndex++;
        }

        if (serviceCenterId > 0) {
            bind[paramIndex].buffer_type = MYSQL_TYPE_LONG;
            bind[paramIndex].buffer = &serviceCenterId;
            paramIndex++;
        }

        if (!status.empty()) {
            bind[paramIndex].buffer_type = MYSQL_TYPE_STRING;
            bind[paramIndex].buffer = (char*)status.c_str();
            bind[paramIndex].buffer_length = status.length();
            paramIndex++;
        }

        if (!city.empty()) {
            std::string likePattern = "%" + city + "%";
            bind[paramIndex].buffer_type = MYSQL_TYPE_STRING;
            bind[paramIndex].buffer = (char*)likePattern.c_str();
            bind[paramIndex].buffer_length = likePattern.length();
            paramIndex++;
        }

        if (!startDate.empty()) {
            bind[paramIndex].buffer_type = MYSQL_TYPE_STRING;
            bind[paramIndex].buffer = (char*)startDate.c_str();
            bind[paramIndex].buffer_length = startDate.length();
            paramIndex++;
        }

        if (!endDate.empty()) {
            bind[paramIndex].buffer_type = MYSQL_TYPE_STRING;
            bind[paramIndex].buffer = (char*)endDate.c_str();
            bind[paramIndex].buffer_length = endDate.length();
            paramIndex++;
        }

        int pageSizeParam = pageSize;
        bind[paramIndex].buffer_type = MYSQL_TYPE_LONG;
        bind[paramIndex].buffer = &pageSizeParam;
        paramIndex++;

        int offsetParam = offset;
        bind[paramIndex].buffer_type = MYSQL_TYPE_LONG;
        bind[paramIndex].buffer = &offsetParam;
        paramIndex++;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        RepairOrder repairOrder;
        MYSQL_BIND resultBind[9];
        memset(resultBind, 0, sizeof(resultBind));

        char statusStr[50] = {0};
        char problemDesc[1000] = {0};
        char expectedFinish[100] = {0};
        char createdAt[100] = {0};
        char updatedAt[100] = {0};
        unsigned long statusLength, problemDescLength, expectedFinishLength, createdAtLength, updatedAtLength;
        bool expectedFinishIsNull = false;

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &repairOrder.id;

        resultBind[1].buffer_type = MYSQL_TYPE_LONG;
        resultBind[1].buffer = &repairOrder.deviceId;

        resultBind[2].buffer_type = MYSQL_TYPE_LONG;
        resultBind[2].buffer = &repairOrder.userId;

        resultBind[3].buffer_type = MYSQL_TYPE_LONG;
        resultBind[3].buffer = &repairOrder.serviceCenterId;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = statusStr;
        resultBind[4].buffer_length = sizeof(statusStr);
        resultBind[4].length = &statusLength;

        resultBind[5].buffer_type = MYSQL_TYPE_STRING;
        resultBind[5].buffer = problemDesc;
        resultBind[5].buffer_length = sizeof(problemDesc);
        resultBind[5].length = &problemDescLength;

        resultBind[6].buffer_type = MYSQL_TYPE_STRING;
        resultBind[6].buffer = expectedFinish;
        resultBind[6].buffer_length = sizeof(expectedFinish);
        resultBind[6].length = &expectedFinishLength;
        resultBind[6].is_null = &expectedFinishIsNull;

        resultBind[7].buffer_type = MYSQL_TYPE_STRING;
        resultBind[7].buffer = createdAt;
        resultBind[7].buffer_length = sizeof(createdAt);
        resultBind[7].length = &createdAtLength;

        resultBind[8].buffer_type = MYSQL_TYPE_STRING;
        resultBind[8].buffer = updatedAt;
        resultBind[8].buffer_length = sizeof(updatedAt);
        resultBind[8].length = &updatedAtLength;

        if (mysql_stmt_bind_result(stmt.get(), resultBind) != 0) {
            throw std::runtime_error(std::string("Failed to bind result: ") + mysql_error(conn));
        }

        std::vector<std::shared_ptr<RepairOrder>> repairOrders;
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            repairOrder.status = RepairOrder::statusFromString(std::string(statusStr, statusLength));
            repairOrder.problemDescription = std::string(problemDesc, problemDescLength);
            if (!expectedFinishIsNull) {
                repairOrder.expectedFinishDate = std::string(expectedFinish, expectedFinishLength);
            }
            repairOrder.createdAt = std::string(createdAt, createdAtLength);
            repairOrder.updatedAt = std::string(updatedAt, updatedAtLength);
            repairOrders.push_back(std::make_shared<RepairOrder>(repairOrder));
        }

        return repairOrders;
    }

    std::shared_ptr<RepairOrder> updateStatus(int id, RepairOrder::Status newStatus, const std::string& note = "") {
        if (id <= 0) {
            return nullptr;
        }

        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "UPDATE repair_orders SET status = ?, updated_at = ? WHERE id = ?";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        std::string statusStr = RepairOrder::statusToString(newStatus);
        std::string now = DateUtils::getToday();

        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)statusStr.c_str();
        bind[0].buffer_length = statusStr.length();

        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)now.c_str();
        bind[1].buffer_length = now.length();

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &id;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        return findById(id);
    }

    std::map<std::string, int> getStatusStatistics() {
        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "SELECT status, COUNT(*) as count FROM repair_orders GROUP BY status";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        MYSQL_BIND resultBind[2];
        memset(resultBind, 0, sizeof(resultBind));

        char status[50] = {0};
        int count = 0;
        unsigned long statusLength;

        resultBind[0].buffer_type = MYSQL_TYPE_STRING;
        resultBind[0].buffer = status;
        resultBind[0].buffer_length = sizeof(status);
        resultBind[0].length = &statusLength;

        resultBind[1].buffer_type = MYSQL_TYPE_LONG;
        resultBind[1].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), resultBind) != 0) {
            throw std::runtime_error(std::string("Failed to bind result: ") + mysql_error(conn));
        }

        std::map<std::string, int> stats;
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            stats[std::string(status, statusLength)] = count;
        }

        return stats;
    }

private:
    RepairOrderRepository() = default;
    RepairOrderRepository(const RepairOrderRepository&) = delete;
    RepairOrderRepository& operator=(const RepairOrderRepository&) = delete;

public:
    static RepairOrderRepository& getInstance() {
        static RepairOrderRepository instance;
        return instance;
    }
};
