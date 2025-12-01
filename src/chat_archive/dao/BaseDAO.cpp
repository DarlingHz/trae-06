#include "chat_archive/dao/BaseDAO.h"
#include "chat_archive/Logger.h"

namespace chat_archive {
namespace dao {

DatabaseResult BaseDAO::execute_query(const std::string& sql, 
                                        const std::vector<std::pair<int, std::string>>& string_params,
                                        const std::vector<std::pair<int, int>>& int_params,
                                        const std::vector<std::pair<int, int64_t>>& int64_params) {
    auto conn = get_connection();
    if (!conn) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get database connection for query");
        return DatabaseResult();
    }
    
    DatabaseQuery query(conn);
    if (!query.prepare(sql)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to prepare query: {}", sql);
        release_connection(conn);
        return DatabaseResult();
    }
    
    // 绑定参数
    for (const auto& param : string_params) {
        if (!query.bind_string(param.first, param.second)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind string parameter at index: {}", param.first);
            release_connection(conn);
            return DatabaseResult();
        }
    }
    
    for (const auto& param : int_params) {
        if (!query.bind_int(param.first, param.second)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind int parameter at index: {}", param.first);
            release_connection(conn);
            return DatabaseResult();
        }
    }
    
    for (const auto& param : int64_params) {
        if (!query.bind_int64(param.first, param.second)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind int64 parameter at index: {}", param.first);
            release_connection(conn);
            return DatabaseResult();
        }
    }
    
    if (!query.execute()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to execute query: {}", sql);
        release_connection(conn);
        return DatabaseResult();
    }
    
    // 获取结果并释放连接
    auto result = query.get_result();
    release_connection(conn);
    
    return result;
}

int64_t BaseDAO::execute_update(const std::string& sql, 
                                  const std::vector<std::pair<int, std::string>>& string_params,
                                  const std::vector<std::pair<int, int>>& int_params,
                                  const std::vector<std::pair<int, int64_t>>& int64_params) {
    auto conn = get_connection();
    if (!conn) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to get database connection for update");
        return -1;
    }
    
    DatabaseQuery query(conn);
    if (!query.prepare(sql)) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to prepare update: {}", sql);
        release_connection(conn);
        return -1;
    }
    
    // 绑定参数
    for (const auto& param : string_params) {
        if (!query.bind_string(param.first, param.second)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind string parameter at index: {}", param.first);
            release_connection(conn);
            return -1;
        }
    }
    
    for (const auto& param : int_params) {
        if (!query.bind_int(param.first, param.second)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind int parameter at index: {}", param.first);
            release_connection(conn);
            return -1;
        }
    }
    
    for (const auto& param : int64_params) {
        if (!query.bind_int64(param.first, param.second)) {
            CHAT_ARCHIVE_LOG_ERROR("Failed to bind int64 parameter at index: {}", param.first);
            release_connection(conn);
            return -1;
        }
    }
    
    if (!query.execute()) {
        CHAT_ARCHIVE_LOG_ERROR("Failed to execute update: {}", sql);
        release_connection(conn);
        return -1;
    }
    
    // 获取受影响的行数并释放连接
    int64_t affected_rows = query.get_affected_rows();
    release_connection(conn);
    
    return affected_rows;
}

} // namespace dao
} // namespace chat_archive