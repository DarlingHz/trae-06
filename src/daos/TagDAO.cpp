#include "daos/TagDAO.hpp"
#include "models/Tag.hpp"
#include "utils/Database.hpp"
#include <iostream>
#include <vector>
#include <map>
#include <string>

namespace daos {

models::Tag TagDAO::rowToTag(const std::map<std::string, std::string>& row) {
    models::Tag tag;
    
    auto it = row.find("id");
    if (it != row.end()) {
        tag.setId(std::stoi(it->second));
    }
    
    it = row.find("name");
    if (it != row.end()) {
        tag.setName(it->second);
    }
    
    it = row.find("created_at");
    if (it != row.end()) {
        tag.setCreatedAt(it->second);
    }
    
    // Tag类中没有updated_at字段，所以不需要处理这个字段
    // 如果需要添加updated_at字段，请修改Tag类的头文件和实现文件
    
    return tag;
}

bool TagDAO::createTag(const models::Tag& tag) {
    std::string sql = "INSERT INTO tags (name, created_at, updated_at) VALUES (?, datetime('now'), datetime('now'))";
    
    std::vector<std::string> params;
    params.push_back(tag.getName());
    
    try {
        bool result = db_.executeWithParams(sql, params);
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error creating tag: " << e.what() << std::endl;
        return false;
    }
}

models::Tag TagDAO::getTagById(int id) {
    std::string sql = "SELECT * FROM tags WHERE id = ?";
    
    std::vector<std::string> params;
    params.push_back(std::to_string(id));
    
    try {
        std::vector<std::map<std::string, std::string>> rows = db_.fetchWithParams(sql, params);
        
        if (!rows.empty()) {
            return rowToTag(rows[0]);
        } else {
            return models::Tag();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting tag by ID: " << e.what() << std::endl;
        return models::Tag();
    }
}

models::Tag TagDAO::getTagByName(const std::string& name) {
    std::string sql = "SELECT * FROM tags WHERE name = ?";
    
    std::vector<std::string> params;
    params.push_back(name);
    
    try {
        std::vector<std::map<std::string, std::string>> rows = db_.fetchWithParams(sql, params);
        
        if (!rows.empty()) {
            return rowToTag(rows[0]);
        } else {
            return models::Tag();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting tag by name: " << e.what() << std::endl;
        return models::Tag();
    }
}

std::vector<models::Tag> TagDAO::getAllTags(int page, int page_size) {
    int offset = (page - 1) * page_size;
    
    std::string sql = "SELECT * FROM tags ORDER BY created_at DESC LIMIT ? OFFSET ?";
    
    std::vector<std::string> params;
    params.push_back(std::to_string(page_size));
    params.push_back(std::to_string(offset));
    
    try {
        std::vector<std::map<std::string, std::string>> rows = db_.fetchWithParams(sql, params);
        
        std::vector<models::Tag> tags;
        for (const auto& row : rows) {
            tags.push_back(rowToTag(row));
        }
        
        return tags;
    } catch (const std::exception& e) {
        std::cerr << "Error getting all tags: " << e.what() << std::endl;
        return std::vector<models::Tag>();
    }
}

bool TagDAO::updateTag(const models::Tag& tag) {
    std::string sql = "UPDATE tags SET name = ?, updated_at = datetime('now') WHERE id = ?";
    
    std::vector<std::string> params;
    params.push_back(tag.getName());
    params.push_back(std::to_string(tag.getId()));
    
    try {
        bool result = db_.executeWithParams(sql, params);
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error updating tag: " << e.what() << std::endl;
        return false;
    }
}

bool TagDAO::deleteTag(int id) {
    std::string sql = "DELETE FROM tags WHERE id = ?";
    
    std::vector<std::string> params;
    params.push_back(std::to_string(id));
    
    try {
        bool result = db_.executeWithParams(sql, params);
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting tag: " << e.what() << std::endl;
        return false;
    }
}

} // namespace daos