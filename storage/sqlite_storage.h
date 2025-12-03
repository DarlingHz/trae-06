#pragma once
#include "storage/storage_interface.h"
#include <sqlite3.h>
#include <string>

namespace storage {

class SQLiteStorage : public StorageInterface {
private:
    sqlite3* db_ = nullptr;
    std::string db_path_;
    
    void create_tables();
    
public:
    explicit SQLiteStorage(const std::string& db_path);
    ~SQLiteStorage() override;
    
    // Prevent copying
    SQLiteStorage(const SQLiteStorage&) = delete;
    SQLiteStorage& operator=(const SQLiteStorage&) = delete;
    
    // Allow moving
    SQLiteStorage(SQLiteStorage&&) noexcept;
    SQLiteStorage& operator=(SQLiteStorage&&) noexcept;
    
    bool initialize() override;
    
    // User operations
    std::optional<domain::User> get_user_by_id(int user_id) const override;
    std::optional<domain::User> get_user_by_role_and_department(const std::string& role, const std::string& department) const override;
    int get_total_users() const override;
    void insert_default_users() override;
    
    // Contract operations
    int create_contract(const domain::Contract& contract) override;
    std::optional<domain::Contract> get_contract_by_id(int contract_id) const override;
    std::vector<domain::Contract> get_contracts(const domain::ContractQueryParams& params) const override;
    bool update_contract(const domain::Contract& contract) override;
    bool update_contract_status(int contract_id, domain::ContractStatus status) override;
    
    // Approval step operations
    std::vector<domain::ApprovalStep> get_approval_steps_by_contract_id(int contract_id) const override;
    std::optional<domain::ApprovalStep> get_current_approval_step(int contract_id) const override;
    int create_approval_step(const domain::ApprovalStep& step) override;
    bool update_approval_step(const domain::ApprovalStep& step) override;
    bool update_approval_step_status(int step_id, domain::ApprovalStepStatus status, int approver_id, const std::optional<std::string>& comment, const std::string& acted_at) override;
    
    // Approval log operations
    int create_approval_log(const domain::ApprovalLog& log) override;
    std::vector<domain::ApprovalLog> get_approval_logs_by_contract_id(int contract_id) const override;
    
    // Pending approvals for user
    std::vector<domain::Contract> get_pending_approvals_for_user(int user_id, int page, int page_size) const override;
};

} // namespace storage
