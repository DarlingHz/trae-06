#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace models {

class Tag {
public:
    // Default constructor
    Tag() = default;

    // Constructor with parameters
    Tag(int id, const std::string& name, const std::string& created_at)
        : id_(id), name_(name), created_at_(created_at) {}

    // Getters
    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    const std::string& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setCreatedAt(const std::string& created_at) { created_at_ = created_at; }

    // Convert Tag object to JSON
    json toJson() const {
        json j;
        j["id"] = id_;
        j["name"] = name_;
        j["created_at"] = created_at_;
        return j;
    }

    // Convert JSON to Tag object
    static Tag fromJson(const json& j) {
        Tag tag;
        tag.id_ = j.value("id", 0);
        tag.name_ = j.value("name", "");
        tag.created_at_ = j.value("created_at", "");
        return tag;
    }

private:
    int id_ = 0;
    std::string name_;
    std::string created_at_;
};

} // namespace models
