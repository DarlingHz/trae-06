#include "repository/SnippetRepository.h"
#include "model/Snippet.h"
#include <sqlite3.h>
#include <string>
#include <optional>
#include <vector>
#include <sstream>
#include <algorithm>
#include <chrono>

namespace repository {

void SnippetRepository::createTables() {
    // 创建代码片段表
    std::string sql_snippets = R"(
        CREATE TABLE IF NOT EXISTS snippets (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_id INTEGER NOT NULL,
            title TEXT NOT NULL,
            language TEXT NOT NULL,
            content TEXT NOT NULL,
            tags TEXT NOT NULL DEFAULT '',
            is_public INTEGER NOT NULL DEFAULT 0,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
            updated_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
            star_count INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (owner_id) REFERENCES users (id) ON DELETE CASCADE
        );
    )";
    execute(sql_snippets);

    // 创建收藏表
    std::string sql_stars = R"(
        CREATE TABLE IF NOT EXISTS stars (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            snippet_id INTEGER NOT NULL,
            user_id INTEGER NOT NULL,
            created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
            FOREIGN KEY (snippet_id) REFERENCES snippets (id) ON DELETE CASCADE,
            FOREIGN KEY (user_id) REFERENCES users (id) ON DELETE CASCADE,
            UNIQUE (snippet_id, user_id)
        );
    )";
    execute(sql_stars);

    // 创建索引
    std::string sql_indexes = R"(
        CREATE INDEX IF NOT EXISTS idx_snippets_owner_id ON snippets (owner_id);
        CREATE INDEX IF NOT EXISTS idx_snippets_is_public ON snippets (is_public);
        CREATE INDEX IF NOT EXISTS idx_snippets_created_at ON snippets (created_at DESC);
        CREATE INDEX IF NOT EXISTS idx_snippets_language ON snippets (language);
        CREATE INDEX IF NOT EXISTS idx_stars_snippet_id ON stars (snippet_id);
        CREATE INDEX IF NOT EXISTS idx_stars_user_id ON stars (user_id);
    )";
    execute(sql_indexes);
}

std::string SnippetRepository::tagsToString(const std::vector<std::string>& tags) {
    std::ostringstream oss;
    for (size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) {
            oss << ',';
        }
        oss << tags[i];
    }
    return oss.str();
}

std::vector<std::string> SnippetRepository::stringToTags(const std::string& tags_str) {
    std::vector<std::string> tags;
    std::istringstream iss(tags_str);
    std::string tag;

    while (std::getline(iss, tag, ',')) {
        if (!tag.empty()) {
            tags.push_back(tag);
        }
    }

    return tags;
}

model::Snippet SnippetRepository::createSnippet(const model::Snippet& snippet) {
    std::string sql = R"(
        INSERT INTO snippets (owner_id, title, language, content, tags, is_public) 
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet.owner_id());
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind owner_id: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_text(stmt, 2, snippet.title().c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind title: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_text(stmt, 3, snippet.language().c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind language: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_text(stmt, 4, snippet.content().c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind content: " + std::string(sqlite3_errmsg(db_)));
    }

    std::string tags_str = tagsToString(snippet.tags());
    rc = sqlite3_bind_text(stmt, 5, tags_str.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind tags: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_int(stmt, 6, snippet.is_public() ? 1 : 0);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind is_public: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 获取插入的 ID
    int snippet_id = sqlite3_last_insert_rowid(db_);

    // 释放资源
    sqlite3_finalize(stmt);

    // 获取当前时间
    auto now = std::chrono::system_clock::now();

    // 返回创建的代码片段对象
    return model::Snippet(snippet_id, snippet.owner_id(), snippet.title(), snippet.language(), 
                           snippet.content(), snippet.tags(), snippet.is_public(), now, now, 0);
}

std::optional<model::Snippet> SnippetRepository::findSnippetById(int snippet_id) {
    std::string sql = R"(
        SELECT id, owner_id, title, language, content, tags, is_public, created_at, updated_at, star_count 
        FROM snippets WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind snippet_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 查询
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        // 获取查询结果
        int id = sqlite3_column_int(stmt, 0);
        int owner_id = sqlite3_column_int(stmt, 1);
        const unsigned char* title_ptr = sqlite3_column_text(stmt, 2);
        const unsigned char* language_ptr = sqlite3_column_text(stmt, 3);
        const unsigned char* content_ptr = sqlite3_column_text(stmt, 4);
        const unsigned char* tags_ptr = sqlite3_column_text(stmt, 5);
        int is_public = sqlite3_column_int(stmt, 6);
        long long created_at_seconds = sqlite3_column_int64(stmt, 7);
        long long updated_at_seconds = sqlite3_column_int64(stmt, 8);
        int star_count = sqlite3_column_int(stmt, 9);

        // 转换数据类型
        std::string title_str(reinterpret_cast<const char*>(title_ptr));
        std::string language_str(reinterpret_cast<const char*>(language_ptr));
        std::string content_str(reinterpret_cast<const char*>(content_ptr));
        std::string tags_str(reinterpret_cast<const char*>(tags_ptr));
        std::vector<std::string> tags = stringToTags(tags_str);
        auto created_at = std::chrono::system_clock::from_time_t(created_at_seconds);
        auto updated_at = std::chrono::system_clock::from_time_t(updated_at_seconds);

        // 释放资源
        sqlite3_finalize(stmt);

        // 返回代码片段对象
        return model::Snippet(id, owner_id, title_str, language_str, content_str, tags, 
                               is_public == 1, created_at, updated_at, star_count);
    } else if (rc == SQLITE_DONE) {
        // 没有找到代码片段
        sqlite3_finalize(stmt);
        return std::nullopt;
    } else {
        // 查询失败
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL query: " + std::string(sqlite3_errmsg(db_)));
    }
}

model::Snippet SnippetRepository::updateSnippet(const model::Snippet& snippet) {
    std::string sql = R"(
        UPDATE snippets SET title = ?, language = ?, content = ?, tags = ?, is_public = ?, updated_at = strftime('%s', 'now') 
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_text(stmt, 1, snippet.title().c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind title: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_text(stmt, 2, snippet.language().c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind language: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_text(stmt, 3, snippet.content().c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind content: " + std::string(sqlite3_errmsg(db_)));
    }

    std::string tags_str = tagsToString(snippet.tags());
    rc = sqlite3_bind_text(stmt, 4, tags_str.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind tags: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_int(stmt, 5, snippet.is_public() ? 1 : 0);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind is_public: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_int(stmt, 6, snippet.id());
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 检查是否有行被更新
    int rows_updated = sqlite3_changes(db_);
    if (rows_updated == 0) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Snippet not found");
    }

    // 释放资源
    sqlite3_finalize(stmt);

    // 获取更新后的代码片段
    auto updated_snippet = findSnippetById(snippet.id());
    if (!updated_snippet) {
        throw DatabaseException("Failed to retrieve updated snippet");
    }

    return *updated_snippet;
}

void SnippetRepository::deleteSnippet(int snippet_id) {
    std::string sql = R"(
        DELETE FROM snippets WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind snippet_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 检查是否有行被删除
    int rows_deleted = sqlite3_changes(db_);
    if (rows_deleted == 0) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Snippet not found");
    }

    // 释放资源
    sqlite3_finalize(stmt);
}

SearchResult SnippetRepository::searchSnippets(const SearchParams& params, int current_user_id) {
    std::string sql;
    std::vector<std::string> where_clauses;
    std::vector<std::pair<int, std::string>> bindings;

    // 基础查询
    sql = R"(
        SELECT id, owner_id, title, language, content, tags, is_public, created_at, updated_at, star_count 
        FROM snippets 
    )";

    // 添加 WHERE 子句
    where_clauses.push_back("(is_public = 1 OR owner_id = ?)");
    bindings.emplace_back(1, std::to_string(current_user_id));

    if (!params.q.empty()) {
        where_clauses.push_back("(title LIKE ? OR content LIKE ?)");
        bindings.emplace_back(2, "%" + params.q + "%");
        bindings.emplace_back(3, "%" + params.q + "%");
    }

    if (!params.language.empty()) {
        where_clauses.push_back("language = ?");
        bindings.emplace_back(4, params.language);
    }

    if (!params.tag.empty()) {
        where_clauses.push_back("tags LIKE ?");
        bindings.emplace_back(5, "%" + params.tag + "%");
    }

    // 组合 WHERE 子句
    if (!where_clauses.empty()) {
        sql += " WHERE ";
        for (size_t i = 0; i < where_clauses.size(); ++i) {
            if (i > 0) {
                sql += " AND ";
            }
            sql += where_clauses[i];
        }
    }

    // 添加 ORDER BY 和 LIMIT
    sql += " ORDER BY created_at DESC LIMIT ? OFFSET ?";
    bindings.emplace_back(6, std::to_string(params.page_size));
    bindings.emplace_back(7, std::to_string((params.page - 1) * params.page_size));

    // 执行查询获取结果
    std::vector<model::Snippet> items;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    for (const auto& binding : bindings) {
        if (binding.second.find_first_not_of("0123456789") == std::string::npos) {
            // 整数参数
            rc = sqlite3_bind_int(stmt, binding.first, std::stoi(binding.second));
        } else {
            // 字符串参数
            rc = sqlite3_bind_text(stmt, binding.first, binding.second.c_str(), -1, SQLITE_STATIC);
        }
        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw DatabaseException("Failed to bind parameter: " + std::string(sqlite3_errmsg(db_)));
        }
    }

    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // 获取查询结果
        int id = sqlite3_column_int(stmt, 0);
        int owner_id = sqlite3_column_int(stmt, 1);
        const unsigned char* title_ptr = sqlite3_column_text(stmt, 2);
        const unsigned char* language_ptr = sqlite3_column_text(stmt, 3);
        const unsigned char* content_ptr = sqlite3_column_text(stmt, 4);
        const unsigned char* tags_ptr = sqlite3_column_text(stmt, 5);
        int is_public = sqlite3_column_int(stmt, 6);
        long long created_at_seconds = sqlite3_column_int64(stmt, 7);
        long long updated_at_seconds = sqlite3_column_int64(stmt, 8);
        int star_count = sqlite3_column_int(stmt, 9);

        // 转换数据类型
        std::string title_str(reinterpret_cast<const char*>(title_ptr));
        std::string language_str(reinterpret_cast<const char*>(language_ptr));
        std::string content_str(reinterpret_cast<const char*>(content_ptr));
        std::string tags_str(reinterpret_cast<const char*>(tags_ptr));
        std::vector<std::string> tags = stringToTags(tags_str);
        auto created_at = std::chrono::system_clock::from_time_t(created_at_seconds);
        auto updated_at = std::chrono::system_clock::from_time_t(updated_at_seconds);

        // 添加到结果列表
        items.emplace_back(id, owner_id, title_str, language_str, content_str, tags, 
                            is_public == 1, created_at, updated_at, star_count);
    }

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL query: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);

    // 获取总记录数
    std::string count_sql = "SELECT COUNT(*) FROM snippets";
    if (!where_clauses.empty()) {
        count_sql += " WHERE ";
        for (size_t i = 0; i < where_clauses.size(); ++i) {
            if (i > 0) {
                count_sql += " AND ";
            }
            count_sql += where_clauses[i];
        }
    }

    int total = 0;
    stmt = nullptr;
    rc = sqlite3_prepare_v2(db_, count_sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare count SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数（排除 LIMIT 和 OFFSET）
    for (const auto& binding : bindings) {
        if (binding.first > 5) { // LIMIT 和 OFFSET 的参数索引是 6 和 7
            break;
        }
        if (binding.second.find_first_not_of("0123456789") == std::string::npos) {
            // 整数参数
            rc = sqlite3_bind_int(stmt, binding.first, std::stoi(binding.second));
        } else {
            // 字符串参数
            rc = sqlite3_bind_text(stmt, binding.first, binding.second.c_str(), -1, SQLITE_STATIC);
        }
        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw DatabaseException("Failed to bind count parameter: " + std::string(sqlite3_errmsg(db_)));
        }
    }

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    } else if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute count SQL query: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);

    // 返回搜索结果
    return SearchResult{items, params.page, params.page_size, total};
}

SearchResult SnippetRepository::getUserSnippets(int user_id, int current_user_id, int page, int page_size) {
    std::string sql;
    std::vector<std::pair<int, std::string>> bindings;

    // 基础查询
    sql = R"(
        SELECT id, owner_id, title, language, content, tags, is_public, created_at, updated_at, star_count 
        FROM snippets 
        WHERE owner_id = ?
    )";
    bindings.emplace_back(1, std::to_string(user_id));

    // 如果不是本人查看，只显示公开的代码片段
    if (current_user_id != user_id) {
        sql += " AND is_public = 1";
    }

    // 添加 ORDER BY 和 LIMIT
    sql += " ORDER BY created_at DESC LIMIT ? OFFSET ?";
    bindings.emplace_back(2, std::to_string(page_size));
    bindings.emplace_back(3, std::to_string((page - 1) * page_size));

    // 执行查询获取结果
    std::vector<model::Snippet> items;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    for (const auto& binding : bindings) {
        if (binding.second.find_first_not_of("0123456789") == std::string::npos) {
            // 整数参数
            rc = sqlite3_bind_int(stmt, binding.first, std::stoi(binding.second));
        } else {
            // 字符串参数
            rc = sqlite3_bind_text(stmt, binding.first, binding.second.c_str(), -1, SQLITE_STATIC);
        }
        if (rc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw DatabaseException("Failed to bind parameter: " + std::string(sqlite3_errmsg(db_)));
        }
    }

    // 执行查询
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        // 获取查询结果
        int id = sqlite3_column_int(stmt, 0);
        int owner_id = sqlite3_column_int(stmt, 1);
        const unsigned char* title_ptr = sqlite3_column_text(stmt, 2);
        const unsigned char* language_ptr = sqlite3_column_text(stmt, 3);
        const unsigned char* content_ptr = sqlite3_column_text(stmt, 4);
        const unsigned char* tags_ptr = sqlite3_column_text(stmt, 5);
        int is_public = sqlite3_column_int(stmt, 6);
        long long created_at_seconds = sqlite3_column_int64(stmt, 7);
        long long updated_at_seconds = sqlite3_column_int64(stmt, 8);
        int star_count = sqlite3_column_int(stmt, 9);

        // 转换数据类型
        std::string title_str(reinterpret_cast<const char*>(title_ptr));
        std::string language_str(reinterpret_cast<const char*>(language_ptr));
        std::string content_str(reinterpret_cast<const char*>(content_ptr));
        std::string tags_str(reinterpret_cast<const char*>(tags_ptr));
        std::vector<std::string> tags = stringToTags(tags_str);
        auto created_at = std::chrono::system_clock::from_time_t(created_at_seconds);
        auto updated_at = std::chrono::system_clock::from_time_t(updated_at_seconds);

        // 添加到结果列表
        items.emplace_back(id, owner_id, title_str, language_str, content_str, tags, 
                            is_public == 1, created_at, updated_at, star_count);
    }

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL query: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);

    // 获取总记录数
    std::string count_sql = "SELECT COUNT(*) FROM snippets WHERE owner_id = ?";
    if (current_user_id != user_id) {
        count_sql += " AND is_public = 1";
    }

    int total = 0;
    stmt = nullptr;
    rc = sqlite3_prepare_v2(db_, count_sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare count SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, user_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind count parameter: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行查询
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        total = sqlite3_column_int(stmt, 0);
    } else if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute count SQL query: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);

    // 返回搜索结果
    return SearchResult{items, page, page_size, total};
}

void SnippetRepository::starSnippet(int snippet_id, int user_id) {
    // 检查是否已经收藏
    if (isSnippetStarred(snippet_id, user_id)) {
        return;
    }

    // 插入收藏记录
    std::string sql = R"(
        INSERT INTO stars (snippet_id, user_id) VALUES (?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind snippet_id: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_int(stmt, 2, user_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind user_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);

    // 更新代码片段的 star_count
    sql = R"(
        UPDATE snippets SET star_count = star_count + 1 WHERE id = ?;
    )";

    stmt = nullptr;
    rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare update SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind snippet_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute update SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);
}

void SnippetRepository::unstarSnippet(int snippet_id, int user_id) {
    // 检查是否已经收藏
    if (!isSnippetStarred(snippet_id, user_id)) {
        return;
    }

    // 删除收藏记录
    std::string sql = R"(
        DELETE FROM stars WHERE snippet_id = ? AND user_id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind snippet_id: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_int(stmt, 2, user_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind user_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);

    // 更新代码片段的 star_count
    sql = R"(
        UPDATE snippets SET star_count = MAX(star_count - 1, 0) WHERE id = ?;
    )";

    stmt = nullptr;
    rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare update SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind snippet_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行 SQL 语句
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute update SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);
}

bool SnippetRepository::isSnippetStarred(int snippet_id, int user_id) {
    std::string sql = R"(
        SELECT COUNT(*) FROM stars WHERE snippet_id = ? AND user_id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        throw DatabaseException("Failed to prepare SQL statement: " + std::string(sqlite3_errmsg(db_)));
    }

    // 绑定参数
    rc = sqlite3_bind_int(stmt, 1, snippet_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind snippet_id: " + std::string(sqlite3_errmsg(db_)));
    }

    rc = sqlite3_bind_int(stmt, 2, user_id);
    if (rc != SQLITE_OK) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to bind user_id: " + std::string(sqlite3_errmsg(db_)));
    }

    // 执行查询
    rc = sqlite3_step(stmt);
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    } else if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw DatabaseException("Failed to execute SQL query: " + std::string(sqlite3_errmsg(db_)));
    }

    // 释放资源
    sqlite3_finalize(stmt);

    return count > 0;
}

} // namespace repository