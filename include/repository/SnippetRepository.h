#pragma once

#include "repository/BaseRepository.h"
#include "model/Snippet.h"
#include <vector>
#include <optional>
#include <string>

namespace repository {

struct SearchParams {
    std::string q;
    std::string language;
    std::string tag;
    int page = 1;
    int page_size = 20;
};

struct SearchResult {
    std::vector<model::Snippet> items;
    int page;
    int page_size;
    int total;
};

class SnippetRepository : public BaseRepository {
public:
    explicit SnippetRepository(const std::string& db_path) : BaseRepository(db_path) {
        createTables();
    }

    // 创建新的代码片段
    model::Snippet createSnippet(const model::Snippet& snippet);

    // 根据 ID 查找代码片段
    std::optional<model::Snippet> findSnippetById(int snippet_id);

    // 更新代码片段
    model::Snippet updateSnippet(const model::Snippet& snippet);

    // 删除代码片段
    void deleteSnippet(int snippet_id);

    // 搜索代码片段
    SearchResult searchSnippets(const SearchParams& params, int current_user_id = -1);

    // 获取某个用户的代码片段
    SearchResult getUserSnippets(int user_id, int current_user_id = -1, int page = 1, int page_size = 20);

    // 收藏代码片段
    void starSnippet(int snippet_id, int user_id);

    // 取消收藏代码片段
    void unstarSnippet(int snippet_id, int user_id);

    // 检查代码片段是否被某个用户收藏
    bool isSnippetStarred(int snippet_id, int user_id);

private:
    // 创建表
    void createTables();

    // 将标签列表转换为数据库存储格式
    std::string tagsToString(const std::vector<std::string>& tags);

    // 将数据库存储格式的标签转换为列表
    std::vector<std::string> stringToTags(const std::string& tags_str);
};

} // namespace repository