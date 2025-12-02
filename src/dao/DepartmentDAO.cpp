#include "dao/DepartmentDAO.h"
#include "logging/Logging.h"
#include <sqlite3.h>

namespace pet_hospital {

bool DepartmentDAO::create_department(const Department& department) {
    try {
        if (!database_) {
            LOG_ERROR("Database connection not initialized");
            return false;
        }

        std::string sql = "INSERT INTO departments (name, description, created_at, updated_at) VALUES (?, ?, datetime('now'), datetime('now'))";
        std::vector<std::vector<std::string>> result;
        int affected_rows;

        if (!database_->execute_statement(sql, affected_rows, {department.get_name(), department.get_description()})) {
            LOG_ERROR("Failed to create department");
            return false;
        }

        LOG_INFO("Department created successfully: " + department.get_name());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create department: " + std::string(e.what()));
        return false;
    }
}

std::optional<Department> DepartmentDAO::get_department_by_id(int department_id) {
    try {
        if (!database_) {
            LOG_ERROR("Database connection not initialized");
            return std::nullopt;
        }

        std::string sql = "SELECT * FROM departments WHERE id = ?";
        std::vector<std::vector<std::string>> result;

        if (!database_->execute_query(sql, result, {std::to_string(department_id)})) {
            LOG_ERROR("Failed to get department by ID");
            return std::nullopt;
        }

        if (result.empty()) {
            LOG_INFO("Department not found: " + std::to_string(department_id));
            return std::nullopt;
        }

        const auto& row = result[0];
        Department department(std::stoi(row[0]), row[1], row[2], row[3], row[4]);

        LOG_INFO("Department retrieved successfully: " + department.get_name());
        return department;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get department by ID: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::vector<Department> DepartmentDAO::get_all_departments() {
    try {
        if (!database_) {
            LOG_ERROR("Database connection not initialized");
            return {};
        }

        std::string sql = "SELECT * FROM departments ORDER BY name";
        std::vector<std::vector<std::string>> result;

        if (!database_->execute_query(sql, result)) {
            LOG_ERROR("Failed to get all departments");
            return {};
        }

        std::vector<Department> departments;
        for (const auto& row : result) {
            departments.emplace_back(std::stoi(row[0]), row[1], row[2], row[3], row[4]);
        }

        LOG_INFO("Departments retrieved successfully: " + std::to_string(departments.size()));
        return departments;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get all departments: " + std::string(e.what()));
        return {};
    }
}

bool DepartmentDAO::update_department(const Department& department) {
    try {
        if (!database_) {
            LOG_ERROR("Database connection not initialized");
            return false;
        }

        std::string sql = "UPDATE departments SET name = ?, description = ?, updated_at = datetime('now') WHERE id = ?";
        std::vector<std::vector<std::string>> result;
        int affected_rows;

        if (!database_->execute_statement(sql, affected_rows, {department.get_name(), department.get_description(), std::to_string(department.get_id())})) {
            LOG_ERROR("Failed to update department");
            return false;
        }

        if (affected_rows == 0) {
            LOG_INFO("Department not found: " + std::to_string(department.get_id()));
            return false;
        }

        LOG_INFO("Department updated successfully: " + department.get_name());
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update department: " + std::string(e.what()));
        return false;
    }
}

bool DepartmentDAO::delete_department(int department_id) {
    try {
        if (!database_) {
            LOG_ERROR("Database connection not initialized");
            return false;
        }

        std::string sql = "DELETE FROM departments WHERE id = " + std::to_string(department_id) + ";";
        std::vector<std::vector<std::string>> result;
        int affected_rows;

        if (!database_->execute_statement(sql, affected_rows)) {
            LOG_ERROR("Failed to delete department");
            return false;
        }

        if (affected_rows == 0) {
            LOG_INFO("Department not found: " + std::to_string(department_id));
            return false;
        }

        LOG_INFO("Department deleted successfully: " + std::to_string(department_id));
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to delete department: " + std::string(e.what()));
        return false;
    }
}

} // namespace pet_hospital