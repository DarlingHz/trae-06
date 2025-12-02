#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace models {

class Album {
public:
    // Default constructor
    Album() = default;

    // Constructor with parameters
    Album(int id, int owner_id, const std::string& title, const std::string& description, const std::string& visibility, const std::string& created_at, const std::string& updated_at)
        : id_(id), owner_id_(owner_id), title_(title), description_(description), visibility_(visibility), created_at_(created_at), updated_at_(updated_at) {}

    // Getters
    int getId() const { return id_; }
    int getOwnerId() const { return owner_id_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getVisibility() const { return visibility_; }
    const std::string& getCreatedAt() const { return created_at_; }
    const std::string& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setOwnerId(int owner_id) { owner_id_ = owner_id; }
    void setTitle(const std::string& title) { title_ = title; }
    void setDescription(const std::string& description) { description_ = description; }
    void setVisibility(const std::string& visibility) { visibility_ = visibility; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::string& updated_at) { updated_at_ = updated_at; }

    // Convert Album object to JSON
    json toJson() const {
        json j;
        j["id"] = id_;
        j["owner_id"] = owner_id_;
        j["title"] = title_;
        j["description"] = description_;
        j["visibility"] = visibility_;
        j["created_at"] = created_at_;
        j["updated_at"] = updated_at_;
        return j;
    }

    // Convert JSON to Album object
    static Album fromJson(const json& j) {
        Album album;
        album.id_ = j.value("id", 0);
        album.owner_id_ = j.value("owner_id", 0);
        album.title_ = j.value("title", "");
        album.description_ = j.value("description", "");
        album.visibility_ = j.value("visibility", "private");
        album.created_at_ = j.value("created_at", "");
        album.updated_at_ = j.value("updated_at", "");
        return album;
    }

private:
    int id_ = 0;
    int owner_id_ = 0;
    std::string title_;
    std::string description_;
    std::string visibility_ = "private";
    std::string created_at_;
    std::string updated_at_;
};

} // namespace models
