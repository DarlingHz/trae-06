#include "dashboard_service.h"
#include "lost_item_service.h"
#include "found_item_service.h"
#include "claim_service.h"
#include "database.h"
#include <stdexcept>

StatData DashboardService::get_stat_data() {
    StatData data;
    
    data.open_lost_items = LostItemService::instance().get_open_lost_items_count();
    data.open_found_items = FoundItemService::instance().get_open_found_items_count();
    data.lost_items_7d = LostItemService::instance().get_lost_items_7d_count();
    data.found_items_7d = FoundItemService::instance().get_found_items_7d_count();
    data.claims_7d = ClaimService::instance().get_claims_7d_count();
    
    return data;
}

std::vector<std::pair<std::string, int>> DashboardService::get_top_categories(int limit) {
    std::vector<std::pair<std::string, int>> result;
    
    // 查询丢失物品分类统计
    std::map<std::string, int> category_counts;
    
    std::string sql_lost = "SELECT category, COUNT(*) FROM lost_items GROUP BY category;";
    Database::instance().execute_query(sql_lost, [&](sqlite3_stmt* stmt) {
        std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int count = sqlite3_column_int(stmt, 1);
        category_counts[category] += count;
        return 0;
    });
    
    // 查询捡到物品分类统计
    std::string sql_found = "SELECT category, COUNT(*) FROM found_items GROUP BY category;";
    Database::instance().execute_query(sql_found, [&](sqlite3_stmt* stmt) {
        std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int count = sqlite3_column_int(stmt, 1);
        category_counts[category] += count;
        return 0;
    });
    
    // 转换为vector并排序
    std::vector<std::pair<std::string, int>> vec(category_counts.begin(), category_counts.end());
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });
    
    // 取前N个
    if (vec.size() > limit) {
        vec.resize(limit);
    }
    
    return vec;
}