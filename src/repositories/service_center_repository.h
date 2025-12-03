#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include "../models/service_center.h"
#include "../db/db_pool.h"
#include "../utils/logger.h"

class ServiceCenterRepository {
public:
    std::shared_ptr<ServiceCenter> create(const ServiceCenter& serviceCenter) {
        if (!serviceCenter.isValid()) {
            throw std::runtime_error("Invalid ServiceCenter data");
        }

        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "INSERT INTO service_centers (name, city, address, contact_phone) VALUES (?, ?, ?, ?)";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        MYSQL_BIND bind[4];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)serviceCenter.name.c_str();
        bind[0].buffer_length = serviceCenter.name.length();

        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)serviceCenter.city.c_str();
        bind[1].buffer_length = serviceCenter.city.length();

        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)serviceCenter.address.c_str();
        bind[2].buffer_length = serviceCenter.address.length();

        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)serviceCenter.contactPhone.c_str();
        bind[3].buffer_length = serviceCenter.contactPhone.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        auto serviceCenterPtr = std::make_shared<ServiceCenter>(serviceCenter);
        serviceCenterPtr->id = mysql_stmt_insert_id(stmt.get());
        return serviceCenterPtr;
    }

    std::shared_ptr<ServiceCenter> findById(int id) {
        if (id <= 0) {
            return nullptr;
        }

        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "SELECT id, name, city, address, contact_phone FROM service_centers WHERE id = ?";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        ServiceCenter serviceCenter;
        MYSQL_BIND resultBind[5];
        memset(resultBind, 0, sizeof(resultBind));

        char name[100] = {0};
        char city[100] = {0};
        char address[500] = {0};
        char contactPhone[20] = {0};
        unsigned long nameLength, cityLength, addressLength, contactPhoneLength;

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &serviceCenter.id;

        resultBind[1].buffer_type = MYSQL_TYPE_STRING;
        resultBind[1].buffer = name;
        resultBind[1].buffer_length = sizeof(name);
        resultBind[1].length = &nameLength;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = city;
        resultBind[2].buffer_length = sizeof(city);
        resultBind[2].length = &cityLength;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = address;
        resultBind[3].buffer_length = sizeof(address);
        resultBind[3].length = &addressLength;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = contactPhone;
        resultBind[4].buffer_length = sizeof(contactPhone);
        resultBind[4].length = &contactPhoneLength;

        if (mysql_stmt_bind_result(stmt.get(), resultBind) != 0) {
            throw std::runtime_error(std::string("Failed to bind result: ") + mysql_error(conn));
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            serviceCenter.name = std::string(name, nameLength);
            serviceCenter.city = std::string(city, cityLength);
            serviceCenter.address = std::string(address, addressLength);
            serviceCenter.contactPhone = std::string(contactPhone, contactPhoneLength);
            return std::make_shared<ServiceCenter>(serviceCenter);
        }

        return nullptr;
    }

    std::vector<std::shared_ptr<ServiceCenter>> findByCity(const std::string& city) {
        auto conn = DBPool::getInstance().getConnection();
        if (!conn) {
            throw std::runtime_error("Failed to get database connection");
        }

        std::string sql = "SELECT id, name, city, address, contact_phone FROM service_centers WHERE city LIKE ?";
        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)> stmt(mysql_stmt_init(conn), mysql_stmt_close);

        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            throw std::runtime_error(std::string("Failed to prepare statement: ") + mysql_error(conn));
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        std::string likePattern = "%" + city + "%";
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)likePattern.c_str();
        bind[0].buffer_length = likePattern.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            throw std::runtime_error(std::string("Failed to bind parameters: ") + mysql_error(conn));
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            throw std::runtime_error(std::string("Failed to execute statement: ") + mysql_error(conn));
        }

        ServiceCenter serviceCenter;
        MYSQL_BIND resultBind[5];
        memset(resultBind, 0, sizeof(resultBind));

        char name[100] = {0};
        char city[100] = {0};
        char address[500] = {0};
        char contactPhone[20] = {0};
        unsigned long nameLength, cityLength, addressLength, contactPhoneLength;

        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &serviceCenter.id;

        resultBind[1].buffer_type = MYSQL_TYPE_STRING;
        resultBind[1].buffer = name;
        resultBind[1].buffer_length = sizeof(name);
        resultBind[1].length = &nameLength;

        resultBind[2].buffer_type = MYSQL_TYPE_STRING;
        resultBind[2].buffer = city;
        resultBind[2].buffer_length = sizeof(city);
        resultBind[2].length = &cityLength;

        resultBind[3].buffer_type = MYSQL_TYPE_STRING;
        resultBind[3].buffer = address;
        resultBind[3].buffer_length = sizeof(address);
        resultBind[3].length = &addressLength;

        resultBind[4].buffer_type = MYSQL_TYPE_STRING;
        resultBind[4].buffer = contactPhone;
        resultBind[4].buffer_length = sizeof(contactPhone);
        resultBind[4].length = &contactPhoneLength;

        if (mysql_stmt_bind_result(stmt.get(), resultBind) != 0) {
            throw std::runtime_error(std::string("Failed to bind result: ") + mysql_error(conn));
        }

        std::vector<std::shared_ptr<ServiceCenter>> serviceCenters;
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            serviceCenter.name = std::string(name, nameLength);
            serviceCenter.city = std::string(city, cityLength);
            serviceCenter.address = std::string(address, addressLength);
            serviceCenter.contactPhone = std::string(contactPhone, contactPhoneLength);
            serviceCenters.push_back(std::make_shared<ServiceCenter>(serviceCenter));
        }

        return serviceCenters;
    }

    bool exists(int id) {
        return findById(id) != nullptr;
    }

private:
    ServiceCenterRepository() = default;
    ServiceCenterRepository(const ServiceCenterRepository&) = delete;
    ServiceCenterRepository& operator=(const ServiceCenterRepository&) = delete;

public:
    static ServiceCenterRepository& getInstance() {
        static ServiceCenterRepository instance;
        return instance;
    }
};
