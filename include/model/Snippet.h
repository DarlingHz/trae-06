#pragma once

#include <string>
#include <vector>
#include <chrono>

namespace model {

class Snippet {
public:
    Snippet() = default;
    Snippet(int id, int owner_id, const std::string& title, const std::string& language, 
            const std::string& content, const std::vector<std::string>& tags, bool is_public, 
            const std::chrono::system_clock::time_point& created_at, 
            const std::chrono::system_clock::time_point& updated_at, int star_count)
        : id_(id), owner_id_(owner_id), title_(title), language_(language), content_(content), 
          tags_(tags), is_public_(is_public), created_at_(created_at), updated_at_(updated_at), 
          star_count_(star_count) {}

    // Getters
    int id() const { return id_; }
    int owner_id() const { return owner_id_; }
    const std::string& title() const { return title_; }
    const std::string& language() const { return language_; }
    const std::string& content() const { return content_; }
    const std::vector<std::string>& tags() const { return tags_; }
    bool is_public() const { return is_public_; }
    const std::chrono::system_clock::time_point& created_at() const { return created_at_; }
    const std::chrono::system_clock::time_point& updated_at() const { return updated_at_; }
    int star_count() const { return star_count_; }

    // Setters
    void set_id(int id) { id_ = id; }
    void set_owner_id(int owner_id) { owner_id_ = owner_id; }
    void set_title(const std::string& title) { title_ = title; }
    void set_language(const std::string& language) { language_ = language; }
    void set_content(const std::string& content) { content_ = content; }
    void set_tags(const std::vector<std::string>& tags) { tags_ = tags; }
    void set_is_public(bool is_public) { is_public_ = is_public; }
    void set_created_at(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    void set_updated_at(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }
    void set_star_count(int star_count) { star_count_ = star_count; }

private:
    int id_ = 0;
    int owner_id_ = 0;
    std::string title_;
    std::string language_;
    std::string content_;
    std::vector<std::string> tags_;
    bool is_public_ = false;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
    int star_count_ = 0;
};

} // namespace model