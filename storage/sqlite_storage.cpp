#include "storage/sqlite_storage.h"
#include <stdexcept>
#include <fmt/format.h>
#include <chrono>
#include <sstream>

namespace storage {

SQLiteStorage::SQLiteStorage(const std::string& db_path) : db_path_(db_path) {}

SQLiteStorage::~SQLiteStorage() {
    if (db_) {
        sqlite3_close(db_);
    }
}

SQLiteStorage::SQLiteStorage(SQLiteStorage&& other) noexcept 
    : db_(other.db_), db_path_(std::move(other.db_path_)) {
    other.db_ = nullptr;
}

SQLiteStorage& SQLiteStorage::operator=(SQLiteStorage&& other) noexcept {
    if (this != &other) {
        if (db_) {
            sqlite3_close(db_);
        }
        db_ = other.db_;
        db_path_ = std::move(other.db_path_);
        other.db_ = nullptr;
    }
    return *this;
}

bool SQLiteStorage::initialize() {
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Cannot open database: {}", sqlite3_errmsg(db_)));
    }
    
    create_tables();
    
    // Insert default users if table is empty
    if (get_total_users() == 0) {
        insert_default_users();
    }
    
    return true;
}

void SQLiteStorage::create_tables() {
    const char* create_users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY,
            name TEXT NOT NULL,
            department TEXT NOT NULL,
            role TEXT NOT NULL
        );
    )";
    
    const char* create_contracts_table = R"(
        CREATE TABLE IF NOT EXISTS contracts (
            id INTEGER PRIMARY KEY,
            title TEXT NOT NULL,
            counterparty TEXT NOT NULL,
            amount INTEGER NOT NULL,
            currency TEXT NOT NULL,
            creator_id INTEGER NOT NULL,
            department TEXT NOT NULL,
            status TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            FOREIGN KEY (creator_id) REFERENCES users(id)
        );
    )";
    
    const char* create_approval_steps_table = R"(
        CREATE TABLE IF NOT EXISTS approval_steps (
            id INTEGER PRIMARY KEY,
            contract_id INTEGER NOT NULL,
            step_order INTEGER NOT NULL,
            role TEXT NOT NULL,
            approver_id INTEGER,
            status TEXT NOT NULL,
            comment TEXT,
            acted_at TEXT,
            FOREIGN KEY (contract_id) REFERENCES contracts(id),
            FOREIGN KEY (approver_id) REFERENCES users(id)
        );
    )";
    
    const char* create_approval_logs_table = R"(
        CREATE TABLE IF NOT EXISTS approval_logs (
            id INTEGER PRIMARY KEY,
            contract_id INTEGER NOT NULL,
            step_id INTEGER,
            operator_id INTEGER NOT NULL,
            action TEXT NOT NULL,
            comment TEXT,
            created_at TEXT NOT NULL,
            FOREIGN KEY (contract_id) REFERENCES contracts(id),
            FOREIGN KEY (step_id) REFERENCES approval_steps(id),
            FOREIGN KEY (operator_id) REFERENCES users(id)
        );
    )";
    
    const char* create_indexes = R"(
        CREATE INDEX IF NOT EXISTS idx_contracts_creator_id ON contracts(creator_id);
        CREATE INDEX IF NOT EXISTS idx_contracts_status ON contracts(status);
        CREATE INDEX IF NOT EXISTS idx_contracts_amount ON contracts(amount);
        CREATE INDEX IF NOT EXISTS idx_contracts_status_creator ON contracts(status, creator_id);
        CREATE INDEX IF NOT EXISTS idx_approval_steps_contract_id ON approval_steps(contract_id);
        CREATE INDEX IF NOT EXISTS idx_approval_steps_approver_id ON approval_steps(approver_id);
        CREATE INDEX IF NOT EXISTS idx_approval_logs_contract_id ON approval_logs(contract_id);
    )";
    
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, create_users_table, nullptr, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(fmt::format("Failed to create users table: {}", error));
    }
    
    rc = sqlite3_exec(db_, create_contracts_table, nullptr, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(fmt::format("Failed to create contracts table: {}", error));
    }
    
    rc = sqlite3_exec(db_, create_approval_steps_table, nullptr, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(fmt::format("Failed to create approval_steps table: {}", error));
    }
    
    rc = sqlite3_exec(db_, create_approval_logs_table, nullptr, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(fmt::format("Failed to create approval_logs table: {}", error));
    }
    
    rc = sqlite3_exec(db_, create_indexes, nullptr, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::string error = err_msg;
        sqlite3_free(err_msg);
        throw std::runtime_error(fmt::format("Failed to create indexes: {}", error));
    }
}

std::optional<domain::User> SQLiteStorage::get_user_by_id(int user_id) const {
    const char* query = "SELECT id, name, department, role FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        domain::User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<domain::User> SQLiteStorage::get_user_by_role_and_department(const std::string& role, const std::string& department) const {
    const char* query = "SELECT id, name, department, role FROM users WHERE role = ? AND department = ? LIMIT 1;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_text(stmt, 1, role.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, department.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        domain::User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        sqlite3_finalize(stmt);
        return user;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

int SQLiteStorage::get_total_users() const {
    const char* query = "SELECT COUNT(*) FROM users;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

void SQLiteStorage::insert_default_users() {
    const char* query = "INSERT INTO users (id, name, department, role) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare insert: {}", sqlite3_errmsg(db_)));
    }
    
    std::vector<std::tuple<int, std::string, std::string, std::string>> users = {
        {1, "张三", "sales", "employee"},
        {2, "李四", "sales", "manager"},
        {3, "王五", "finance", "finance_approver"},
        {4, "赵六", "legal", "legal_approver"}
    };
    
    for (const auto& user : users) {
        sqlite3_bind_int(stmt, 1, std::get<0>(user));
        sqlite3_bind_text(stmt, 2, std::get<1>(user).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, std::get<2>(user).c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, std::get<3>(user).c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error(fmt::format("Failed to insert user: {}", sqlite3_errmsg(db_)));
        }
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
}

int SQLiteStorage::create_contract(const domain::Contract& contract) {
    const char* query = "INSERT INTO contracts (title, counterparty, amount, currency, creator_id, department, status, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare insert: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_text(stmt, 1, contract.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contract.counterparty.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, contract.amount);
    sqlite3_bind_text(stmt, 4, contract.currency.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, contract.creator_id);
    sqlite3_bind_text(stmt, 6, contract.department.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, domain::Contract::status_to_string(contract.status).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 8, contract.created_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, contract.updated_at.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to insert contract: {}", sqlite3_errmsg(db_)));
    }
    
    int contract_id = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return contract_id;
}

std::optional<domain::Contract> SQLiteStorage::get_contract_by_id(int contract_id) const {
    const char* query = "SELECT id, title, counterparty, amount, currency, creator_id, department, status, created_at, updated_at FROM contracts WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, contract_id);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        domain::Contract contract;
        contract.id = sqlite3_column_int(stmt, 0);
        contract.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        contract.counterparty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        contract.amount = sqlite3_column_int64(stmt, 3);
        contract.currency = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        contract.creator_id = sqlite3_column_int(stmt, 5);
        contract.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        contract.status = domain::Contract::string_to_status(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        contract.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        contract.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        
        sqlite3_finalize(stmt);
        return contract;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<domain::Contract> SQLiteStorage::get_contracts(const domain::ContractQueryParams& params) const {
    std::ostringstream query;
    query << "SELECT id, title, counterparty, amount, currency, creator_id, department, status, created_at, updated_at FROM contracts WHERE 1=1";
    
    if (params.status) {
        query << " AND status = ?";
    }
    if (params.creator_id) {
        query << " AND creator_id = ?";
    }
    if (params.min_amount) {
        query << " AND amount >= ?";
    }
    if (params.max_amount) {
        query << " AND amount <= ?";
    }
    
    query << " ORDER BY created_at DESC LIMIT ? OFFSET ?";
    
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    int param_index = 1;
    if (params.status) {
        sqlite3_bind_text(stmt, param_index++, params.status->c_str(), -1, SQLITE_STATIC);
    }
    if (params.creator_id) {
        sqlite3_bind_int(stmt, param_index++, *params.creator_id);
    }
    if (params.min_amount) {
        sqlite3_bind_int64(stmt, param_index++, *params.min_amount);
    }
    if (params.max_amount) {
        sqlite3_bind_int64(stmt, param_index++, *params.max_amount);
    }
    
    sqlite3_bind_int(stmt, param_index++, params.page_size);
    sqlite3_bind_int(stmt, param_index++, (params.page - 1) * params.page_size);
    
    std::vector<domain::Contract> contracts;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        domain::Contract contract;
        contract.id = sqlite3_column_int(stmt, 0);
        contract.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        contract.counterparty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        contract.amount = sqlite3_column_int64(stmt, 3);
        contract.currency = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        contract.creator_id = sqlite3_column_int(stmt, 5);
        contract.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        contract.status = domain::Contract::string_to_status(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        contract.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        contract.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        
        contracts.push_back(contract);
    }
    
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to execute query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_finalize(stmt);
    return contracts;
}

bool SQLiteStorage::update_contract(const domain::Contract& contract) {
    const char* query = "UPDATE contracts SET title = ?, counterparty = ?, amount = ?, currency = ?, department = ?, status = ?, updated_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare update: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_text(stmt, 1, contract.title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contract.counterparty.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 3, contract.amount);
    sqlite3_bind_text(stmt, 4, contract.currency.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, contract.department.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, domain::Contract::status_to_string(contract.status).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, contract.updated_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, contract.id);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to update contract: {}", sqlite3_errmsg(db_)));
    }
    
    int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return changes > 0;
}

bool SQLiteStorage::update_contract_status(int contract_id, domain::ContractStatus status) {
    const char* query = "UPDATE contracts SET status = ?, updated_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare update: {}", sqlite3_errmsg(db_)));
    }
    
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&now_time_t), "%Y-%m-%dT%H:%M:%SZ");
    std::string updated_at = ss.str();
    
    sqlite3_bind_text(stmt, 1, domain::Contract::status_to_string(status).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, updated_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, contract_id);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to update contract status: {}", sqlite3_errmsg(db_)));
    }
    
    int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return changes > 0;
}

std::vector<domain::ApprovalStep> SQLiteStorage::get_approval_steps_by_contract_id(int contract_id) const {
    const char* query = "SELECT id, contract_id, step_order, role, approver_id, status, comment, acted_at FROM approval_steps WHERE contract_id = ? ORDER BY step_order;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, contract_id);
    
    std::vector<domain::ApprovalStep> steps;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        domain::ApprovalStep step;
        step.id = sqlite3_column_int(stmt, 0);
        step.contract_id = sqlite3_column_int(stmt, 1);
        step.step_order = sqlite3_column_int(stmt, 2);
        step.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            step.approver_id = sqlite3_column_int(stmt, 4);
        }
        
        step.status = domain::ApprovalStep::string_to_status(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            step.comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        }
        
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            step.acted_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }
        
        steps.push_back(step);
    }
    
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to execute query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_finalize(stmt);
    return steps;
}

std::optional<domain::ApprovalStep> SQLiteStorage::get_current_approval_step(int contract_id) const {
    const char* query = "SELECT id, contract_id, step_order, role, approver_id, status, comment, acted_at FROM approval_steps WHERE contract_id = ? AND status = ? ORDER BY step_order LIMIT 1;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, contract_id);
    sqlite3_bind_text(stmt, 2, "pending", -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        domain::ApprovalStep step;
        step.id = sqlite3_column_int(stmt, 0);
        step.contract_id = sqlite3_column_int(stmt, 1);
        step.step_order = sqlite3_column_int(stmt, 2);
        step.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        
        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            step.approver_id = sqlite3_column_int(stmt, 4);
        }
        
        step.status = domain::ApprovalStep::string_to_status(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        
        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            step.comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        }
        
        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            step.acted_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }
        
        sqlite3_finalize(stmt);
        return step;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

int SQLiteStorage::create_approval_step(const domain::ApprovalStep& step) {
    const char* query = "INSERT INTO approval_steps (contract_id, step_order, role, approver_id, status, comment, acted_at) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare insert: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, step.contract_id);
    sqlite3_bind_int(stmt, 2, step.step_order);
    sqlite3_bind_text(stmt, 3, step.role.c_str(), -1, SQLITE_STATIC);
    
    if (step.approver_id) {
        sqlite3_bind_int(stmt, 4, *step.approver_id);
    } else {
        sqlite3_bind_null(stmt, 4);
    }
    
    sqlite3_bind_text(stmt, 5, domain::ApprovalStep::status_to_string(step.status).c_str(), -1, SQLITE_STATIC);
    
    if (step.comment) {
        sqlite3_bind_text(stmt, 6, step.comment->c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 6);
    }
    
    if (step.acted_at) {
        sqlite3_bind_text(stmt, 7, step.acted_at->c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 7);
    }
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to insert approval step: {}", sqlite3_errmsg(db_)));
    }
    
    int step_id = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return step_id;
}

bool SQLiteStorage::update_approval_step(const domain::ApprovalStep& step) {
    const char* query = "UPDATE approval_steps SET approver_id = ?, status = ?, comment = ?, acted_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare update: {}", sqlite3_errmsg(db_)));
    }
    
    if (step.approver_id) {
        sqlite3_bind_int(stmt, 1, *step.approver_id);
    } else {
        sqlite3_bind_null(stmt, 1);
    }
    
    sqlite3_bind_text(stmt, 2, domain::ApprovalStep::status_to_string(step.status).c_str(), -1, SQLITE_STATIC);
    
    if (step.comment) {
        sqlite3_bind_text(stmt, 3, step.comment->c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 3);
    }
    
    if (step.acted_at) {
        sqlite3_bind_text(stmt, 4, step.acted_at->c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 4);
    }
    
    sqlite3_bind_int(stmt, 5, step.id);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to update approval step: {}", sqlite3_errmsg(db_)));
    }
    
    int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return changes > 0;
}

bool SQLiteStorage::update_approval_step_status(int step_id, domain::ApprovalStepStatus status, int approver_id, const std::optional<std::string>& comment, const std::string& acted_at) {
    const char* query = "UPDATE approval_steps SET status = ?, approver_id = ?, comment = ?, acted_at = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare update: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_text(stmt, 1, domain::ApprovalStep::status_to_string(status).c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, approver_id);
    
    if (comment) {
        sqlite3_bind_text(stmt, 3, comment->c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 3);
    }
    
    sqlite3_bind_text(stmt, 4, acted_at.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, step_id);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to update approval step status: {}", sqlite3_errmsg(db_)));
    }
    
    int changes = sqlite3_changes(db_);
    sqlite3_finalize(stmt);
    return changes > 0;
}

int SQLiteStorage::create_approval_log(const domain::ApprovalLog& log) {
    const char* query = "INSERT INTO approval_logs (contract_id, step_id, operator_id, action, comment, created_at) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare insert: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, log.contract_id);
    
    if (log.step_id) {
        sqlite3_bind_int(stmt, 2, *log.step_id);
    } else {
        sqlite3_bind_null(stmt, 2);
    }
    
    sqlite3_bind_int(stmt, 3, log.operator_id);
    sqlite3_bind_text(stmt, 4, domain::ApprovalLog::action_to_string(log.action).c_str(), -1, SQLITE_STATIC);
    
    if (log.comment) {
        sqlite3_bind_text(stmt, 5, log.comment->c_str(), -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, 5);
    }
    
    sqlite3_bind_text(stmt, 6, log.created_at.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to insert approval log: {}", sqlite3_errmsg(db_)));
    }
    
    int log_id = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return log_id;
}

std::vector<domain::ApprovalLog> SQLiteStorage::get_approval_logs_by_contract_id(int contract_id) const {
    const char* query = "SELECT id, contract_id, step_id, operator_id, action, comment, created_at FROM approval_logs WHERE contract_id = ? ORDER BY created_at;";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, contract_id);
    
    std::vector<domain::ApprovalLog> logs;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        domain::ApprovalLog log;
        log.id = sqlite3_column_int(stmt, 0);
        log.contract_id = sqlite3_column_int(stmt, 1);
        
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            log.step_id = sqlite3_column_int(stmt, 2);
        }
        
        log.operator_id = sqlite3_column_int(stmt, 3);
        log.action = domain::ApprovalLog::string_to_action(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        
        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
            log.comment = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        }
        
        log.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        
        logs.push_back(log);
    }
    
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to execute query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_finalize(stmt);
    return logs;
}

std::vector<domain::Contract> SQLiteStorage::get_pending_approvals_for_user(int user_id, int page, int page_size) const {
    const char* query = R"(
        SELECT c.id, c.title, c.counterparty, c.amount, c.currency, c.creator_id, c.department, c.status, c.created_at, c.updated_at
        FROM contracts c
        JOIN approval_steps a ON c.id = a.contract_id
        WHERE a.approver_id = ? AND a.status = ? AND c.status = ?
        GROUP BY c.id
        ORDER BY c.created_at DESC
        LIMIT ? OFFSET ?
    )";
    
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db_, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(fmt::format("Failed to prepare query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, "pending", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, "approving", -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, page_size);
    sqlite3_bind_int(stmt, 5, (page - 1) * page_size);
    
    std::vector<domain::Contract> contracts;
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        domain::Contract contract;
        contract.id = sqlite3_column_int(stmt, 0);
        contract.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        contract.counterparty = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        contract.amount = sqlite3_column_int64(stmt, 3);
        contract.currency = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        contract.creator_id = sqlite3_column_int(stmt, 5);
        contract.department = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        contract.status = domain::Contract::string_to_status(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        contract.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        contract.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        
        contracts.push_back(contract);
    }
    
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error(fmt::format("Failed to execute query: {}", sqlite3_errmsg(db_)));
    }
    
    sqlite3_finalize(stmt);
    return contracts;
}

} // namespace storage
