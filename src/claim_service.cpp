#include "claim_service.h"
#include "database.h"
#include "lost_item_service.h"
#include "found_item_service.h"
#include "notification_service.h"
#include "logger.h"
#include <stdexcept>

std::optional<ClaimDTO> ClaimService::create_claim(const CreateClaimRequest& request, int user_id) {
    // 检查物品是否存在且状态合法
    auto lost_item = LostItemService::instance().get_lost_item_by_id(request.lost_item_id);
    auto found_item = FoundItemService::instance().get_found_item_by_id(request.found_item_id);
    
    if (!lost_item.has_value() || !found_item.has_value()) {
        return std::nullopt;
    }
    
    if (lost_item->status != "open" || found_item->status != "open") {
        return std::nullopt;
    }
    
    // 检查是否已有approved的认领记录
    std::string sql_check = "SELECT COUNT(*) FROM claims WHERE lost_item_id = " + std::to_string(request.lost_item_id) + 
                          " AND found_item_id = " + std::to_string(request.found_item_id) + " AND status = 'approved';";
    
    int count = 0;
    Database::instance().execute_query(sql_check, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    if (count > 0) {
        return std::nullopt;
    }
    
    // 开始事务
    if (!Database::instance().transaction_start()) {
        return std::nullopt;
    }
    
    std::string sql = "INSERT INTO claims (lost_item_id, found_item_id, claimant_user_id, evidence_text) VALUES (";
    sql += std::to_string(request.lost_item_id) + "," + std::to_string(request.found_item_id) + "," + 
           std::to_string(user_id) + ",'" + request.evidence_text + "');";
    
    int claim_id = 0;
    if (!Database::instance().execute_update(sql, &claim_id)) {
        Database::instance().transaction_rollback();
        return std::nullopt;
    }
    
    // 提交事务
    if (!Database::instance().transaction_commit()) {
        return std::nullopt;
    }
    
    // 创建通知
    NotificationService::instance().create_notification(lost_item->owner_user_id, 
        "您的丢失物品有新的认领申请", "claim_created");
    
    return get_claim_by_id(claim_id);
}

std::vector<ClaimDTO> ClaimService::get_claims(int user_id, const std::optional<std::string>& status) {
    std::vector<ClaimDTO> claims;
    
    std::string sql = "SELECT id, lost_item_id, found_item_id, claimant_user_id, status, evidence_text, created_at, updated_at FROM claims WHERE ";
    sql += "claimant_user_id = " + std::to_string(user_id);
    
    if (status.has_value()) {
        sql += " AND status = '" + status.value() + "'";
    }
    
    sql += " ORDER BY created_at DESC;";
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        ClaimDTO claim;
        claim.id = sqlite3_column_int(stmt, 0);
        claim.lost_item_id = sqlite3_column_int(stmt, 1);
        claim.found_item_id = sqlite3_column_int(stmt, 2);
        claim.claimant_user_id = sqlite3_column_int(stmt, 3);
        claim.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        claim.evidence_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        claim.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        claim.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        
        claims.push_back(claim);
        return 0;
    });
    
    return claims;
}

std::optional<ClaimDTO> ClaimService::get_claim_by_id(int id) {
    std::string sql = "SELECT id, lost_item_id, found_item_id, claimant_user_id, status, evidence_text, created_at, updated_at FROM claims WHERE id = " + std::to_string(id) + ";";
    
    ClaimDTO claim;
    bool found = false;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        claim.id = sqlite3_column_int(stmt, 0);
        claim.lost_item_id = sqlite3_column_int(stmt, 1);
        claim.found_item_id = sqlite3_column_int(stmt, 2);
        claim.claimant_user_id = sqlite3_column_int(stmt, 3);
        claim.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        claim.evidence_text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        claim.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        claim.updated_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        
        found = true;
        return 1;
    });
    
    return found ? std::optional<ClaimDTO>(claim) : std::nullopt;
}

bool ClaimService::approve_claim(int id, const UserDTO& admin_user) {
    if (admin_user.role != "admin" && admin_user.role != "staff") {
        return false;
    }
    
    auto claim = get_claim_by_id(id);
    if (!claim.has_value() || claim->status != "pending") {
        return false;
    }
    
    // 开始事务
    if (!Database::instance().transaction_start()) {
        return false;
    }
    
    // 检查是否已有approved的认领记录
    std::string sql_check = "SELECT COUNT(*) FROM claims WHERE lost_item_id = " + std::to_string(claim->lost_item_id) + 
                          " AND found_item_id = " + std::to_string(claim->found_item_id) + " AND status = 'approved';";
    
    int count = 0;
    Database::instance().execute_query(sql_check, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    if (count > 0) {
        Database::instance().transaction_rollback();
        return false;
    }
    
    // 更新认领状态为approved
    std::string sql_claim = "UPDATE claims SET status = 'approved', updated_at = CURRENT_TIMESTAMP WHERE id = " + std::to_string(id) + ";";
    if (!Database::instance().execute_update(sql_claim)) {
        Database::instance().transaction_rollback();
        return false;
    }
    
    // 更新丢失物品状态为matched
    std::string sql_lost = "UPDATE lost_items SET status = 'matched', updated_at = CURRENT_TIMESTAMP WHERE id = " + std::to_string(claim->lost_item_id) + ";";
    if (!Database::instance().execute_update(sql_lost)) {
        Database::instance().transaction_rollback();
        return false;
    }
    
    // 更新捡到物品状态为matched
    std::string sql_found = "UPDATE found_items SET status = 'matched', updated_at = CURRENT_TIMESTAMP WHERE id = " + std::to_string(claim->found_item_id) + ";";
    if (!Database::instance().execute_update(sql_found)) {
        Database::instance().transaction_rollback();
        return false;
    }
    
    // 提交事务
    if (!Database::instance().transaction_commit()) {
        return false;
    }
    
    // 创建通知
    NotificationService::instance().create_notification(claim->claimant_user_id, 
        "您的认领申请已通过", "claim_approved");
    
    return true;
}

bool ClaimService::reject_claim(int id, const UserDTO& admin_user) {
    if (admin_user.role != "admin" && admin_user.role != "staff") {
        return false;
    }
    
    auto claim = get_claim_by_id(id);
    if (!claim.has_value() || claim->status != "pending") {
        return false;
    }
    
    std::string sql = "UPDATE claims SET status = 'rejected', updated_at = CURRENT_TIMESTAMP WHERE id = " + std::to_string(id) + ";";
    
    if (!Database::instance().execute_update(sql)) {
        return false;
    }
    
    // 创建通知
    NotificationService::instance().create_notification(claim->claimant_user_id, 
        "您的认领申请已被拒绝", "claim_rejected");
    
    return true;
}

bool ClaimService::is_claim_possible(int lost_item_id, int found_item_id) {
    auto lost_item = LostItemService::instance().get_lost_item_by_id(lost_item_id);
    auto found_item = FoundItemService::instance().get_found_item_by_id(found_item_id);
    
    if (!lost_item.has_value() || !found_item.has_value()) {
        return false;
    }
    
    if (lost_item->status != "open" || found_item->status != "open") {
        return false;
    }
    
    return true;
}

int ClaimService::get_claims_7d_count() {
    std::string sql = "SELECT COUNT(*) FROM claims WHERE created_at >= datetime('now', '-7 days');";
    int count = 0;
    
    Database::instance().execute_query(sql, [&](sqlite3_stmt* stmt) {
        count = sqlite3_column_int(stmt, 0);
        return 1;
    });
    
    return count;
}