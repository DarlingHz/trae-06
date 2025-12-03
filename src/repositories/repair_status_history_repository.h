#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include "../models/repair_status_history.h"
#include "../db/db_pool.h"
#include "../utils/logger.h"
#include "../utils/date_utils.h"

class RepairStatusHistoryRepository {
public:
    std::shared_ptr<RepairStatusHistory> create(const RepairStatusHistory& history) {
        if (!history.isValid()) {
            throw std::runtime_error("Invalid RepairStatusHistory data");
        }

        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "INSERT INTO repair_status_history (repair_order_id, status, note, operator, created_at) "
                          "VALUES (?, ?, ?, ?, ?)";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        std::string now = DateUtils::getToday();

        MYSQL_BIND bind[5];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &history.repairOrderId;

        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)history.status.c_str();
        bind[1].buffer_length = history.status.length();

        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = history.note.empty() ? nullptr : (char*)history.note.c_str();
        bind[2].buffer_length = history.note.length();
        bind[2].is_null = history.note.empty() ? &is_null : nullptr;

        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = history.operatorUser.empty() ? nullptr : (char*)history.operatorUser.c_str();
        bind[3].buffer_length = history.operatorUser.length();
        bind[3].is_null = history.operatorUser.empty() ? &is_null : nullptr;

        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)now.c_str();
        bind[4].buffer_length = now.length();

        bool is_null = true;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        auto historyPtr = std::make_shared<RepairStatusHistory>(history);
        historyPtr->id = mysql_stmt_insert_id(stmt.get());
        historyPtr->createdAt = now;
        return historyPtr;
    }

    std::vector<std::shared_ptr<RepairStatusHistory>> findByRepairOrderId(int repairOrderId) {
        if (repairOrderId <= 0) {
            return {};
        }

        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "SELECT id, repair_order_id, status, note, operator, created_at "
                          "FROM repair_status_history WHERE repair_order_id = ? ORDER BY created_at DESC";
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
        bind[0].buffer = &repairOrderId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        RepairStatusHistory history;
        MYSQL_BIND resultBind[6];
        memset(resultBind, 0, sizeof(resultBind));

        char status[50] = {0};
        char note[500] = {0};
        char operatorUser[100] = {0};
        char createdAt[100] = {0};
        unsigned long statusLength, noteLength, operatorUserLength, createdAtLength;
        bool noteIsNull = false;
        bool operatorIsNull = false;

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &history.id;

        resultBind[1].buffer_type = MYSQL_TYPE_LONG;
        resultBind[1].buffer = &history.repairOrderId;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = status;
        resultBind[2].buffer_length = sizeof(status);
        resultBind[2].length = &statusLength;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = note;
        resultBind[3].buffer_length = sizeof(note);
        resultBind[3].length = &noteLength;
        resultBind[3].is_null = &noteIsNull;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = operatorUser;
        resultBind[4].buffer_length = sizeof(operatorUser);
        resultBind[4].length = &operatorUserLength;
        resultBind[4].is_null = &operatorIsNull;

        resultBind[5].buffer_type = MYSQL_TYPE_STRING;
        resultBind[5].buffer = createdAt;
        resultBind[5].buffer_length = sizeof(createdAt);
        resultBind[5].length = &createdAtLength;

        if (mysql_stmt_bind_result(stmt.get(), resultBind) != 0) {
            throw std::runtime_error(std::string("Failed to bind result: ") + mysql_error(conn));
        }

        std::vector<std::shared_ptr<RepairStatusHistory>> historyList;
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            history.status = std::string(status, statusLength);
            if (!noteIsNull) {
                history.note = std::string(note, noteLength);
            }
            if (!operatorIsNull) {
                history.operatorUser = std::string(operatorUser, operatorUserLength);
            }
            history.createdAt = std::string(createdAt, createdAtLength);
            historyList.push_back(std::make_shared<RepairStatusHistory>(history));
        }

        return historyList;
    }

private:
    RepairStatusHistoryRepository() = default;
    RepairStatusHistoryRepository(const RepairStatusHistoryRepository&) = delete;
    RepairStatusHistoryRepository& operator=(const RepairStatusHistoryRepository&) = delete;

public:
    static RepairStatusHistoryRepository& getInstance() {
        static RepairStatusHistoryRepository instance;
        return instance;
    }
};
