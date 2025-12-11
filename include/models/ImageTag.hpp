#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace models {

class ImageTag {
public:
    // Default constructor
    ImageTag() = default;

    // Constructor with parameters
    ImageTag(int id, int image_id, int tag_id)
        : id_(id), image_id_(image_id), tag_id_(tag_id) {}

    // Getters
    int getId() const { return id_; }
    int getImageId() const { return image_id_; }
    int getTagId() const { return tag_id_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setImageId(int image_id) { image_id_ = image_id; }
    void setTagId(int tag_id) { tag_id_ = tag_id; }

    // Convert ImageTag object to JSON
    json toJson() const {
        json j;
        j["id"] = id_;
        j["image_id"] = image_id_;
        j["tag_id"] = tag_id_;
        return j;
    }

    // Convert JSON to ImageTag object
    static ImageTag fromJson(const json& j) {
        ImageTag image_tag;
        image_tag.id_ = j.value("id", 0);
        image_tag.image_id_ = j.value("image_id", 0);
        image_tag.tag_id_ = j.value("tag_id", 0);
        return image_tag;
    }

private:
    int id_ = 0;
    int image_id_ = 0;
    int tag_id_ = 0;
};

} // namespace models
