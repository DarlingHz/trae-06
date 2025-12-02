#ifndef SNIPPET_SERVICE_H
#define SNIPPET_SERVICE_H

#include "repository/SnippetRepository.h"
#include "ServiceException.h"
#include "model/Snippet.h"
#include <string>
#include <optional>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>

namespace service {

class SnippetService {
public:
    struct SearchParams {
        std::string q; // 搜索关键字
        std::string language; // 按语言过滤
        std::string tag; // 按标签过滤
        int page; // 页码
        int page_size; // 每页大小

        SearchParams() : page(1), page_size(20) {}
    };

    struct SearchResult {
        std::vector<model::Snippet> items; // 搜索结果列表
        int page; // 当前页码
        int page_size; // 每页大小
        int total; // 总记录数
    };

    SnippetService(repository::SnippetRepository& snippet_repository);
    ~SnippetService() = default;

    // 禁止拷贝构造和赋值运算符
    SnippetService(const SnippetService&) = delete;
    SnippetService& operator=(const SnippetService&) = delete;

    // 禁止移动构造和赋值运算符，因为 std::mutex 是不可移动的
    SnippetService(SnippetService&&) noexcept = delete;
    SnippetService& operator=(SnippetService&&) noexcept = delete;

    /**
     * @brief 创建代码片段
     * @param snippet 代码片段对象
     * @param current_user_id 当前用户ID
     * @return 创建成功的代码片段对象
     * @throws ServiceException 如果参数不合法或创建失败
     */
    model::Snippet createSnippet(const model::Snippet& snippet, int current_user_id);

    /**
     * @brief 获取单个代码片段详情
     * @param snippet_id 代码片段ID
     * @param current_user_id 当前用户ID（可选，用于权限检查）
     * @return 代码片段对象
     * @throws ServiceException 如果代码片段不存在或没有权限访问
     */
    model::Snippet getSnippetById(int snippet_id, std::optional<int> current_user_id = std::nullopt);

    /**
     * @brief 更新代码片段
     * @param snippet 代码片段对象
     * @param current_user_id 当前用户ID
     * @return 更新成功的代码片段对象
     * @throws ServiceException 如果参数不合法、代码片段不存在或没有权限更新
     */
    model::Snippet updateSnippet(const model::Snippet& snippet, int current_user_id);

    /**
     * @brief 删除代码片段
     * @param snippet_id 代码片段ID
     * @param current_user_id 当前用户ID
     * @throws ServiceException 如果代码片段不存在或没有权限删除
     */
    void deleteSnippet(int snippet_id, int current_user_id);

    /**
     * @brief 搜索代码片段
     * @param params 搜索参数
     * @param current_user_id 当前用户ID
     * @return 搜索结果
     * @throws ServiceException 如果搜索失败
     */
    SearchResult searchSnippets(const SearchParams& params, int current_user_id);

    /**
     * @brief 获取某个用户的代码片段
     * @param user_id 用户ID
     * @param current_user_id 当前用户ID
     * @param page 页码
     * @param page_size 每页大小
     * @return 搜索结果
     * @throws ServiceException 如果获取失败
     */
    SearchResult getUserSnippets(int user_id, int current_user_id, int page = 1, int page_size = 20);

    /**
     * @brief 收藏代码片段
     * @param snippet_id 代码片段ID
     * @param current_user_id 当前用户ID
     * @throws ServiceException 如果代码片段不存在或收藏失败
     */
    void starSnippet(int snippet_id, int current_user_id);

    /**
     * @brief 取消收藏代码片段
     * @param snippet_id 代码片段ID
     * @param current_user_id 当前用户ID
     * @throws ServiceException 如果代码片段不存在或取消收藏失败
     */
    void unstarSnippet(int snippet_id, int current_user_id);

    /**
     * @brief 检查代码片段是否被当前用户收藏
     * @param snippet_id 代码片段ID
     * @param current_user_id 当前用户ID
     * @return 如果已收藏返回true，否则返回false
     * @throws ServiceException 如果代码片段不存在
     */
    bool isSnippetStarred(int snippet_id, int current_user_id);

private:
    /**
     * @brief 检查用户是否有权限访问代码片段
     * @param snippet 代码片段对象
     * @param current_user_id 当前用户ID（可选）
     * @throws ServiceException 如果没有权限访问
     */
    void checkSnippetAccess(const model::Snippet& snippet, std::optional<int> current_user_id);

    /**
     * @brief 检查用户是否有权限修改代码片段
     * @param snippet 代码片段对象
     * @param current_user_id 当前用户ID
     * @throws ServiceException 如果没有权限修改
     */
    void checkSnippetModification(const model::Snippet& snippet, int current_user_id);

    /**
     * @brief 验证代码片段参数是否合法
     * @param snippet 代码片段对象
     * @throws ServiceException 如果参数不合法
     */
    void validateSnippetParams(const model::Snippet& snippet);

private:
    repository::SnippetRepository& snippet_repository_;

    // 热点代码片段缓存
    struct CachedSnippet {
        model::Snippet snippet;
        std::chrono::system_clock::time_point last_access_time;
        int access_count;
    };

    std::unordered_map<int, CachedSnippet> snippet_cache_; // 代码片段ID到缓存对象的映射
    std::mutex cache_mutex_; // 保护snippet_cache_的互斥锁
    static const int MAX_CACHE_SIZE = 100; // 缓存的最大大小
};

} // namespace service

#endif // SNIPPET_SERVICE_H