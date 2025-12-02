#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace models {

class ImageItem {
public:
    // Default constructor
    ImageItem() = default;

    // Constructor with parameters
    ImageItem(int id, int album_id, int owner_id, const std::string& image_url, const std::string& title, const std::string& description, const std::string& source_page_url, const std::string& created_at)
        : id_(id), album_id_(album_id), owner_id_(owner_id), image_url_(image_url), title_(title), description_(description), source_page_url_(source_page_url), created_at_(created_at) {}

    // Getters
    int getId() const { return id_; }
    int getAlbumId() const { return album_id_; }
    int getOwnerId() const { return owner_id_; }
    const std::string& getImageUrl() const { return image_url_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getSourcePageUrl() const { return source_page_url_; }
    const std::string& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setAlbumId(int album_id) { album_id_ = album_id; }
    void setOwnerId(int owner_id) { owner_id_ = owner_id; }
    void setImageUrl(const std::string& image_url) { image_url_ = image_url; }
    void setTitle(const std::string& title) { title_ = title; }
    void setDescription(const std::string& description) { description_ = description; }
    void setSourcePageUrl(const std::string& source_page_url) { source_page_url_ = source_page_url; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

    // Convert ImageItem object to JSON
    json toJson() const {
        json j;
        j["id"] = id_;
        j["album_id"] = album_id_;
        j["owner_id"] = owner_id_;
        j["image_url"] = image_url_;
        j["title"] = title_;
        j["description"] = description_;
        j["source_page_url"] = source_page_url_;
        j["created_at"] = created_at_;
        return j;
    }

    // Convert JSON to ImageItem object
    static ImageItem fromJson(const json& j) {
        ImageItem image_item;
        image_item.id_ = j.value("id", 0);
        image_item.album_id_ = j.value("album_id", 0);
        image_item.owner_id_ = j.value("owner_id", 0);
        image_item.image_url_ = j.value("image_url", "");
        image_item.title_ = j.value("title", "");
        image_item.description_ = j.value("description", "");
        image_item.source_page_url_ = j.value("source_page_url", "");
        image_item.created_at_ = j.value("created_at", "");
        return image_item;
    }

private:
    int id_ = 0;
    int album_id_ = 0;
    int owner_id_ = 0;
    std::string image_url_;
    std::string title_;
    std::string description_;
    std::string source_page_url_;
    std::string created_at_;
};

} // namespace models
