#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <map>
#include "../models/device.h"
#include "../db/db_pool.h"
#include "../utils/date_utils.h"
#include "../utils/logger.h"

class DeviceRepository {
public:
    std::shared_ptr<Device> create(const Device& device) {
        if (!device.isValid()) {
            throw std::runtime_error("Invalid device data");
        }

        // 检查序列号是否已存在
        if (findBySerialNumber(device.serialNumber)) {
            throw std::runtime_error("Serial number already exists");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "INSERT INTO devices (owner_user_id, category, brand, model, serial_number, purchase_date, warranty_expire_at, created_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?)";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        std::string categoryStr = Device::categoryToString(device.category);
        time_t now = time(nullptr);
        MYSQL_BIND params[8] = {0};

        params[0].buffer_type = MYSQL_TYPE_LONG;
        params[0].buffer = const_cast<int*>(&device.ownerUserId);

        params[1].buffer_type = MYSQL_TYPE_STRING;
        params[1].buffer = const_cast<char*>(categoryStr.c_str());
        params[1].buffer_length = categoryStr.length();

        params[2].buffer_type = MYSQL_TYPE_STRING;
        params[2].buffer = const_cast<char*>(device.brand.c_str());
        params[2].buffer_length = device.brand.length();

        params[3].buffer_type = MYSQL_TYPE_STRING;
        params[3].buffer = const_cast<char*>(device.model.c_str());
        params[3].buffer_length = device.model.length();

        params[4].buffer_type = MYSQL_TYPE_STRING;
        params[4].buffer = const_cast<char*>(device.serialNumber.c_str());
        params[4].buffer_length = device.serialNumber.length();

        params[5].buffer_type = MYSQL_TYPE_LONG;
        params[5].buffer = const_cast<time_t*>(&device.purchaseDate);

        params[6].buffer_type = MYSQL_TYPE_LONG;
        params[6].buffer = const_cast<time_t*>(&device.warrantyExpireAt);

        params[7].buffer_type = MYSQL_TYPE_LONG;
        params[7].buffer = &now;

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        int64_t deviceId = mysql_stmt_insert_id(stmt);
        mysql_stmt_close(stmt);

        return std::make_shared<Device>(deviceId, device.ownerUserId, device.category, device.brand,
                                       device.model, device.serialNumber, device.purchaseDate,
                                       device.warrantyExpireAt, now);
    }

    std::shared_ptr<Device> findById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid device ID");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT id, owner_user_id, category, brand, model, serial_number, purchase_date, warranty_expire_at, created_at FROM devices WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        MYSQL_BIND params[1] = {0};
        params[0].buffer_type = MYSQL_TYPE_LONG;
        params[0].buffer = &id;

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        MYSQL_BIND result[9] = {0};
        int deviceId;
        int ownerUserId;
        char category[256] = {0};
        char brand[256] = {0};
        char model[256] = {0};
        char serialNumber[256] = {0};
        time_t purchaseDate;
        time_t warrantyExpireAt;
        time_t createdAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &deviceId;

        result[1].buffer_type = MYSQL_TYPE_LONG;
        result[1].buffer = &ownerUserId;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = category;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_STRING;
        result[3].buffer = brand;
        result[3].buffer_length = 256;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = model;
        result[4].buffer_length = 256;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = serialNumber;
        result[5].buffer_length = 256;

        result[6].buffer_type = MYSQL_TYPE_LONG;
        result[6].buffer = &purchaseDate;

        result[7].buffer_type = MYSQL_TYPE_LONG;
        result[7].buffer = &warrantyExpireAt;

        result[8].buffer_type = MYSQL_TYPE_LONG;
        result[8].buffer = &createdAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::shared_ptr<Device> device;
        if (mysql_stmt_fetch(stmt) == 0) {
            Device::Category categoryEnum = Device::categoryFromString(std::string(category));
            device = std::make_shared<Device>(deviceId, ownerUserId, categoryEnum, std::string(brand),
                                             std::string(model), std::string(serialNumber), purchaseDate,
                                             warrantyExpireAt, createdAt);
        }

        mysql_stmt_close(stmt);
        return device;
    }

    std::shared_ptr<Device> findBySerialNumber(const std::string& serialNumber) {
        if (serialNumber.empty()) {
            throw std::runtime_error("Invalid serial number");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT id, owner_user_id, category, brand, model, serial_number, purchase_date, warranty_expire_at, created_at FROM devices WHERE serial_number = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        MYSQL_BIND params[1] = {0};
        params[0].buffer_type = MYSQL_TYPE_STRING;
        params[0].buffer = const_cast<char*>(serialNumber.c_str());
        params[0].buffer_length = serialNumber.length();

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        MYSQL_BIND result[9] = {0};
        int deviceId;
        int ownerUserId;
        char category[256] = {0};
        char brand[256] = {0};
        char model[256] = {0};
        char serialNumberResult[256] = {0};
        time_t purchaseDate;
        time_t warrantyExpireAt;
        time_t createdAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &deviceId;

        result[1].buffer_type = MYSQL_TYPE_LONG;
        result[1].buffer = &ownerUserId;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = category;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_STRING;
        result[3].buffer = brand;
        result[3].buffer_length = 256;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = model;
        result[4].buffer_length = 256;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = serialNumberResult;
        result[5].buffer_length = 256;

        result[6].buffer_type = MYSQL_TYPE_LONG;
        result[6].buffer = &purchaseDate;

        result[7].buffer_type = MYSQL_TYPE_LONG;
        result[7].buffer = &warrantyExpireAt;

        result[8].buffer_type = MYSQL_TYPE_LONG;
        result[8].buffer = &createdAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::shared_ptr<Device> device;
        if (mysql_stmt_fetch(stmt) == 0) {
            Device::Category categoryEnum = Device::categoryFromString(std::string(category));
            device = std::make_shared<Device>(deviceId, ownerUserId, categoryEnum, std::string(brand),
                                             std::string(model), std::string(serialNumberResult), purchaseDate,
                                             warrantyExpireAt, createdAt);
        }

        mysql_stmt_close(stmt);
        return device;
    }

    std::vector<std::shared_ptr<Device>> findByUserId(int userId, int page = 1, int pageSize = 10, const std::string& category = "", bool underWarranty = false) {
        if (userId <= 0) {
            throw std::runtime_error("Invalid user ID");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT id, owner_user_id, category, brand, model, serial_number, purchase_date, warranty_expire_at, created_at FROM devices WHERE owner_user_id = ?";

        std::vector<std::string> conditions;
        if (!category.empty()) {
            query += " AND category = ?";
            conditions.push_back(category);
        }

        if (underWarranty) {
            query += " AND warranty_expire_at > UNIX_TIMESTAMP()";
        }

        query += " ORDER BY created_at DESC LIMIT ? OFFSET ?";

        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        std::vector<MYSQL_BIND> params;
        MYSQL_BIND param = {0};

        // User ID parameter
        param.buffer_type = MYSQL_TYPE_LONG;
        param.buffer = const_cast<int*>(&userId);
        params.push_back(param);

        // Category parameter
        if (!category.empty()) {
            param = {0};
            param.buffer_type = MYSQL_TYPE_STRING;
            param.buffer = const_cast<char*>(category.c_str());
            param.buffer_length = category.length();
            params.push_back(param);
        }

        // Page size parameter
        param = {0};
        param.buffer_type = MYSQL_TYPE_LONG;
        int pageSizeVal = pageSize;
        param.buffer = &pageSizeVal;
        params.push_back(param);

        // Offset parameter
        param = {0};
        param.buffer_type = MYSQL_TYPE_LONG;
        int offset = (page - 1) * pageSize;
        param.buffer = &offset;
        params.push_back(param);

        if (mysql_stmt_bind_param(stmt, params.data()) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        MYSQL_BIND result[9] = {0};
        int deviceId;
        int ownerUserId;
        char categoryResult[256] = {0};
        char brand[256] = {0};
        char model[256] = {0};
        char serialNumber[256] = {0};
        time_t purchaseDate;
        time_t warrantyExpireAt;
        time_t createdAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &deviceId;

        result[1].buffer_type = MYSQL_TYPE_LONG;
        result[1].buffer = &ownerUserId;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = categoryResult;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_STRING;
        result[3].buffer = brand;
        result[3].buffer_length = 256;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = model;
        result[4].buffer_length = 256;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = serialNumber;
        result[5].buffer_length = 256;

        result[6].buffer_type = MYSQL_TYPE_LONG;
        result[6].buffer = &purchaseDate;

        result[7].buffer_type = MYSQL_TYPE_LONG;
        result[7].buffer = &warrantyExpireAt;

        result[8].buffer_type = MYSQL_TYPE_LONG;
        result[8].buffer = &createdAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::vector<std::shared_ptr<Device>> devices;
        while (mysql_stmt_fetch(stmt) == 0) {
            Device::Category categoryEnum = Device::categoryFromString(std::string(categoryResult));
            auto device = std::make_shared<Device>(deviceId, ownerUserId, categoryEnum, std::string(brand),
                                                   std::string(model), std::string(serialNumber), purchaseDate,
                                                   warrantyExpireAt, createdAt);
            devices.push_back(device);
        }

        mysql_stmt_close(stmt);
        return devices;
    }

    void updateWarrantyExpireAt(int deviceId, time_t newExpireAt) {
        if (deviceId <= 0 || newExpireAt <= 0) {
            throw std::runtime_error("Invalid device ID or expiration date");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "UPDATE devices SET warranty_expire_at = ? WHERE id = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        MYSQL_BIND params[2] = {0};

        params[0].buffer_type = MYSQL_TYPE_LONG;
        params[0].buffer = const_cast<time_t*>(&newExpireAt);

        params[1].buffer_type = MYSQL_TYPE_LONG;
        params[1].buffer = const_cast<int*>(&deviceId);

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        mysql_stmt_close(stmt);
    }

    std::vector<std::shared_ptr<Device>> findWarrantyUpcoming(int userId, int days = 30) {
        if (userId <= 0 || days <= 0) {
            throw std::runtime_error("Invalid user ID or days");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        time_t today = DateUtils::getToday();
        time_t endDate = DateUtils::addDays(today, days);

        std::string query = "SELECT id, owner_user_id, category, brand, model, serial_number, purchase_date, warranty_expire_at, created_at FROM devices WHERE owner_user_id = ? AND warranty_expire_at BETWEEN ? AND ? ORDER BY warranty_expire_at ASC";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        MYSQL_BIND params[3] = {0};

        params[0].buffer_type = MYSQL_TYPE_LONG;
        params[0].buffer = const_cast<int*>(&userId);

        params[1].buffer_type = MYSQL_TYPE_LONG;
        params[1].buffer = &today;

        params[2].buffer_type = MYSQL_TYPE_LONG;
        params[2].buffer = &endDate;

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind parameters: " + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to execute statement: " + err);
        }

        MYSQL_BIND result[9] = {0};
        int deviceId;
        int ownerUserId;
        char category[256] = {0};
        char brand[256] = {0};
        char model[256] = {0};
        char serialNumber[256] = {0};
        time_t purchaseDate;
        time_t warrantyExpireAt;
        time_t createdAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &deviceId;

        result[1].buffer_type = MYSQL_TYPE_LONG;
        result[1].buffer = &ownerUserId;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = category;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_STRING;
        result[3].buffer = brand;
        result[3].buffer_length = 256;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = model;
        result[4].buffer_length = 256;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = serialNumber;
        result[5].buffer_length = 256;

        result[6].buffer_type = MYSQL_TYPE_LONG;
        result[6].buffer = &purchaseDate;

        result[7].buffer_type = MYSQL_TYPE_LONG;
        result[7].buffer = &warrantyExpireAt;

        result[8].buffer_type = MYSQL_TYPE_LONG;
        result[8].buffer = &createdAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::vector<std::shared_ptr<Device>> devices;
        while (mysql_stmt_fetch(stmt) == 0) {
            Device::Category categoryEnum = Device::categoryFromString(std::string(category));
            auto device = std::make_shared<Device>(deviceId, ownerUserId, categoryEnum, std::string(brand),
                                                   std::string(model), std::string(serialNumber), purchaseDate,
                                                   warrantyExpireAt, createdAt);
            devices.push_back(device);
        }

        mysql_stmt_close(stmt);
        return devices;
    }

private:
    DeviceRepository(const DeviceRepository&) = delete;
    DeviceRepository& operator=(const DeviceRepository&) = delete;

public:
    static DeviceRepository& getInstance() {
        static DeviceRepository instance;
        return instance;
    }
};
