#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace models {

class ImageLike {
public:
    // Default constructor
    ImageLike() = default;

    // Constructor with parameters
    ImageLike(int id, int image_id, int user_id, const std::string& created_at)
        : id_(id), image_id_(image_id), user_id_(user_id), created_at_(created_at) {}

    // Getters
    int getId() const { return id_; }
    int getImageId() const { return image_id_; }
    int getUserId() const { return user_id_; }
    const std::string& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setImageId(int image_id) { image_id_ = image_id; }
    void setUserId(int user_id) { user_id_ = user_id; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

    // Convert ImageLike object to JSON
    json toJson() const {
        json j;
        j["id"] = id_;
        j["image_id"] = image_id_;
        j["user_id"] = user_id_;
        j["created_at"] = created_at_;
        return j;
    }

    // Convert JSON to ImageLike object
    static ImageLike fromJson(const json& j) {
        ImageLike image_like;
        image_like.id_ = j.value("id", 0);
        image_like.image_id_ = j.value("image_id", 0);
        image_like.user_id_ = j.value("user_id", 0);
        image_like.created_at_ = j.value("created_at", "");
        return image_like;
    }

private:
    int id_ = 0;
    int image_id_ = 0;
    int user_id_ = 0;
    std::string created_at_;
};

} // namespace models
