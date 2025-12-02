#include "service/SnippetService.h"
#include <stdexcept>
#include <algorithm>
#include <chrono>

namespace service {

SnippetService::SnippetService(repository::SnippetRepository& snippet_repository)
    : snippet_repository_(snippet_repository) {
}

model::Snippet SnippetService::createSnippet(const model::Snippet& snippet, int current_user_id) {
    // 验证参数
    validateSnippetParams(snippet);

    // 创建新的代码片段对象，设置所有者ID
    model::Snippet new_snippet = snippet;
    new_snippet.set_owner_id(current_user_id);

    try {
        // 调用 Repository 层的方法创建代码片段
        model::Snippet created_snippet = snippet_repository_.createSnippet(new_snippet);

        // 将新创建的代码片段添加到缓存
        { // 加锁保护snippet_cache_
            std::lock_guard<std::mutex> lock(cache_mutex_);

            // 如果缓存已满，移除最近最少使用的代码片段
            if (snippet_cache_.size() >= MAX_CACHE_SIZE) {
                auto lru_it = std::min_element(snippet_cache_.begin(), snippet_cache_.end(),
                    [](const auto& a, const auto& b) {
                        if (a.second.last_access_time != b.second.last_access_time) {
                            return a.second.last_access_time < b.second.last_access_time;
                        }
                        return a.second.access_count < b.second.access_count;
                    });
                snippet_cache_.erase(lru_it);
            }

            // 添加新的代码片段到缓存
            snippet_cache_[created_snippet.id()] = {
                created_snippet,
                std::chrono::system_clock::now(),
                1
            };
        }

        return created_snippet;
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to create snippet: " + std::string(e.what()));
    }
}

model::Snippet SnippetService::getSnippetById(int snippet_id, std::optional<int> current_user_id) {
    model::Snippet snippet;
    bool is_cached = false;

    // 首先从缓存中获取代码片段
    { // 加锁保护snippet_cache_
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = snippet_cache_.find(snippet_id);
        if (it != snippet_cache_.end()) {
            snippet = it->second.snippet;
            is_cached = true;

            // 更新缓存中的访问时间和访问次数
            it->second.last_access_time = std::chrono::system_clock::now();
            it->second.access_count++;
        }
    }

    // 如果缓存中没有，从数据库中获取
    if (!is_cached) {
        try {
            auto optional_snippet = snippet_repository_.findSnippetById(snippet_id);
            if (!optional_snippet) {
                throw ServiceException("Snippet not found");
            }
            snippet = *optional_snippet;

            // 将从数据库中获取的代码片段添加到缓存
            { // 加锁保护snippet_cache_
                std::lock_guard<std::mutex> lock(cache_mutex_);

                // 如果缓存已满，移除最近最少使用的代码片段
                if (snippet_cache_.size() >= MAX_CACHE_SIZE) {
                    auto lru_it = std::min_element(snippet_cache_.begin(), snippet_cache_.end(),
                        [](const auto& a, const auto& b) {
                            if (a.second.last_access_time != b.second.last_access_time) {
                                return a.second.last_access_time < b.second.last_access_time;
                            }
                            return a.second.access_count < b.second.access_count;
                        });
                    snippet_cache_.erase(lru_it);
                }

                // 添加新的代码片段到缓存
                snippet_cache_[snippet.id()] = {
                    snippet,
                    std::chrono::system_clock::now(),
                    1
                };
            }
        } catch (const repository::DatabaseException& e) {
            throw ServiceException("Failed to get snippet: " + std::string(e.what()));
        }
    }

    // 检查用户是否有权限访问代码片段
    checkSnippetAccess(snippet, current_user_id);

    return snippet;
}

model::Snippet SnippetService::updateSnippet(const model::Snippet& snippet, int current_user_id) {
    // 验证参数
    validateSnippetParams(snippet);

    // 检查代码片段是否存在
    model::Snippet existing_snippet = getSnippetById(snippet.id(), current_user_id);

    // 检查用户是否有权限修改代码片段
    checkSnippetModification(existing_snippet, current_user_id);

    try {
        // 调用 Repository 层的方法更新代码片段
        model::Snippet updated_snippet = snippet_repository_.updateSnippet(snippet);

        // 更新缓存中的代码片段
        { // 加锁保护snippet_cache_
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto it = snippet_cache_.find(updated_snippet.id());
            if (it != snippet_cache_.end()) {
                it->second.snippet = updated_snippet;
                it->second.last_access_time = std::chrono::system_clock::now();
                it->second.access_count++;
            }
        }

        return updated_snippet;
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to update snippet: " + std::string(e.what()));
    }
}

void SnippetService::deleteSnippet(int snippet_id, int current_user_id) {
    // 检查代码片段是否存在
    model::Snippet existing_snippet = getSnippetById(snippet_id, current_user_id);

    // 检查用户是否有权限删除代码片段
    checkSnippetModification(existing_snippet, current_user_id);

    try {
        // 调用 Repository 层的方法删除代码片段
        snippet_repository_.deleteSnippet(snippet_id);

        // 从缓存中移除代码片段
        { // 加锁保护snippet_cache_
            std::lock_guard<std::mutex> lock(cache_mutex_);
            snippet_cache_.erase(snippet_id);
        }
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to delete snippet: " + std::string(e.what()));
    }
}

SnippetService::SearchResult SnippetService::searchSnippets(const SearchParams& params, int current_user_id) {
    try {
        // 调用 Repository 层的方法搜索代码片段
        repository::SearchParams repo_params;
        repo_params.q = params.q;
        repo_params.language = params.language;
        repo_params.tag = params.tag;
        repo_params.page = params.page;
        repo_params.page_size = params.page_size;

        repository::SearchResult repo_result = 
            snippet_repository_.searchSnippets(repo_params, current_user_id);

        // 转换为 Service 层的搜索结果
        SearchResult result;
        result.items = repo_result.items;
        result.page = repo_result.page;
        result.page_size = repo_result.page_size;
        result.total = repo_result.total;

        return result;
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to search snippets: " + std::string(e.what()));
    }
}

SnippetService::SearchResult SnippetService::getUserSnippets(int user_id, int current_user_id, int page, int page_size) {
    try {
        // 调用 Repository 层的方法获取用户的代码片段
        repository::SearchResult repo_result = 
            snippet_repository_.getUserSnippets(user_id, current_user_id, page, page_size);

        // 转换为 Service 层的搜索结果
        SearchResult result;
        result.items = repo_result.items;
        result.page = repo_result.page;
        result.page_size = repo_result.page_size;
        result.total = repo_result.total;

        return result;
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to get user snippets: " + std::string(e.what()));
    }
}

void SnippetService::starSnippet(int snippet_id, int current_user_id) {
    // 检查代码片段是否存在
    model::Snippet existing_snippet = getSnippetById(snippet_id, current_user_id);

    try {
        // 调用 Repository 层的方法收藏代码片段
        snippet_repository_.starSnippet(snippet_id, current_user_id);

        // 更新缓存中的代码片段的 star_count
        { // 加锁保护snippet_cache_
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto it = snippet_cache_.find(snippet_id);
            if (it != snippet_cache_.end()) {
                model::Snippet updated_snippet = it->second.snippet;
                updated_snippet.set_star_count(updated_snippet.star_count() + 1);
                it->second.snippet = updated_snippet;
                it->second.last_access_time = std::chrono::system_clock::now();
                it->second.access_count++;
            }
        }
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to star snippet: " + std::string(e.what()));
    }
}

void SnippetService::unstarSnippet(int snippet_id, int current_user_id) {
    // 检查代码片段是否存在
    model::Snippet existing_snippet = getSnippetById(snippet_id, current_user_id);

    try {
        // 调用 Repository 层的方法取消收藏代码片段
        snippet_repository_.unstarSnippet(snippet_id, current_user_id);

        // 更新缓存中的代码片段的 star_count
        { // 加锁保护snippet_cache_
            std::lock_guard<std::mutex> lock(cache_mutex_);
            auto it = snippet_cache_.find(snippet_id);
            if (it != snippet_cache_.end()) {
                model::Snippet updated_snippet = it->second.snippet;
                updated_snippet.set_star_count(std::max(updated_snippet.star_count() - 1, 0));
                it->second.snippet = updated_snippet;
                it->second.last_access_time = std::chrono::system_clock::now();
                it->second.access_count++;
            }
        }
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to unstar snippet: " + std::string(e.what()));
    }
}

bool SnippetService::isSnippetStarred(int snippet_id, int current_user_id) {
    // 检查代码片段是否存在
    model::Snippet existing_snippet = getSnippetById(snippet_id, current_user_id);

    try {
        // 调用 Repository 层的方法检查代码片段是否被收藏
        return snippet_repository_.isSnippetStarred(snippet_id, current_user_id);
    } catch (const repository::DatabaseException& e) {
        throw ServiceException("Failed to check if snippet is starred: " + std::string(e.what()));
    }
}

void SnippetService::checkSnippetAccess(const model::Snippet& snippet, std::optional<int> current_user_id) {
    // 如果代码片段是公开的，任何人都可以访问
    if (snippet.is_public()) {
        return;
    }

    // 如果代码片段是私有的，只有所有者可以访问
    if (!current_user_id || *current_user_id != snippet.owner_id()) {
        throw ServiceException("Access denied");
    }
}

void SnippetService::checkSnippetModification(const model::Snippet& snippet, int current_user_id) {
    // 只有代码片段的所有者可以修改或删除
    if (current_user_id != snippet.owner_id()) {
        throw ServiceException("Access denied");
    }
}

void SnippetService::validateSnippetParams(const model::Snippet& snippet) {
    // 验证标题
    if (snippet.title().empty()) {
        throw ServiceException("Title cannot be empty");
    }
    if (snippet.title().size() > 100) {
        throw ServiceException("Title cannot exceed 100 characters");
    }

    // 验证语言
    if (snippet.language().empty()) {
        throw ServiceException("Language cannot be empty");
    }
    if (snippet.language().size() > 50) {
        throw ServiceException("Language cannot exceed 50 characters");
    }

    // 验证内容
    if (snippet.content().empty()) {
        throw ServiceException("Content cannot be empty");
    }

    // 验证标签
    for (const auto& tag : snippet.tags()) {
        if (tag.empty()) {
            throw ServiceException("Tag cannot be empty");
        }
        if (tag.size() > 50) {
            throw ServiceException("Tag cannot exceed 50 characters");
        }
    }
}

} // namespace service