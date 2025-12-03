#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include "../models/warranty_policy.h"
#include "../db/db_pool.h"
#include "../utils/logger.h"

class WarrantyPolicyRepository {
public:
    std::shared_ptr<WarrantyPolicy> create(const WarrantyPolicy& policy) {
        if (!policy.isValid()) {
            throw std::runtime_error("Invalid warranty policy data");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "INSERT INTO warranty_policies (device_id, provider_name, policy_type, coverage_desc, expire_at) VALUES (?, ?, ?, ?, ?)";
        MYSQL_STMT* stmt = mysql_stmt_init(mysqlConn);
        if (!stmt) {
            throw std::runtime_error("Failed to initialize statement");
        }

        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            std::string err = mysql_error(mysqlConn);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to prepare statement: " + err);
        }

        std::string policyTypeStr = WarrantyPolicy::policyTypeToString(policy.policyType);
        MYSQL_BIND params[5] = {0};

        params[0].buffer_type = MYSQL_TYPE_LONG;
        params[0].buffer = const_cast<int*>(&policy.deviceId);

        params[1].buffer_type = MYSQL_TYPE_STRING;
        params[1].buffer = const_cast<char*>(policy.providerName.c_str());
        params[1].buffer_length = policy.providerName.length();

        params[2].buffer_type = MYSQL_TYPE_STRING;
        params[2].buffer = const_cast<char*>(policyTypeStr.c_str());
        params[2].buffer_length = policyTypeStr.length();

        params[3].buffer_type = MYSQL_TYPE_STRING;
        params[3].buffer = const_cast<char*>(policy.coverageDesc.c_str());
        params[3].buffer_length = policy.coverageDesc.length();

        params[4].buffer_type = MYSQL_TYPE_LONG;
        params[4].buffer = const_cast<time_t*>(&policy.expireAt);

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

        int64_t policyId = mysql_stmt_insert_id(stmt);
        mysql_stmt_close(stmt);

        return std::make_shared<WarrantyPolicy>(policyId, policy.deviceId, policy.providerName,
                                               policy.policyType, policy.coverageDesc, policy.expireAt);
    }

    std::shared_ptr<WarrantyPolicy> findById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid warranty policy ID");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT id, device_id, provider_name, policy_type, coverage_desc, expire_at FROM warranty_policies WHERE id = ?";
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

        MYSQL_BIND result[6] = {0};
        int policyId;
        int deviceId;
        char providerName[256] = {0};
        char policyType[256] = {0};
        char coverageDesc[512] = {0};
        time_t expireAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &policyId;

        result[1].buffer_type = MYSQL_TYPE_LONG;
        result[1].buffer = &deviceId;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = providerName;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_STRING;
        result[3].buffer = policyType;
        result[3].buffer_length = 256;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = coverageDesc;
        result[4].buffer_length = 512;

        result[5].buffer_type = MYSQL_TYPE_LONG;
        result[5].buffer = &expireAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::shared_ptr<WarrantyPolicy> policy;
        if (mysql_stmt_fetch(stmt) == 0) {
            WarrantyPolicy::PolicyType policyTypeEnum = WarrantyPolicy::policyTypeFromString(std::string(policyType));
            policy = std::make_shared<WarrantyPolicy>(policyId, deviceId, std::string(providerName),
                                                     policyTypeEnum, std::string(coverageDesc), expireAt);
        }

        mysql_stmt_close(stmt);
        return policy;
    }

    std::vector<std::shared_ptr<WarrantyPolicy>> findByDeviceId(int deviceId) {
        if (deviceId <= 0) {
            throw std::runtime_error("Invalid device ID");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT id, device_id, provider_name, policy_type, coverage_desc, expire_at FROM warranty_policies WHERE device_id = ? ORDER BY expire_at DESC";
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
        params[0].buffer = const_cast<int*>(&deviceId);

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

        MYSQL_BIND result[6] = {0};
        int policyId;
        int resultDeviceId;
        char providerName[256] = {0};
        char policyType[256] = {0};
        char coverageDesc[512] = {0};
        time_t expireAt;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &policyId;

        result[1].buffer_type = MYSQL_TYPE_LONG;
        result[1].buffer = &resultDeviceId;

        result[2].buffer_type = MYSQL_TYPE_STRING;
        result[2].buffer = providerName;
        result[2].buffer_length = 256;

        result[3].buffer_type = MYSQL_TYPE_STRING;
        result[3].buffer = policyType;
        result[3].buffer_length = 256;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = coverageDesc;
        result[4].buffer_length = 512;

        result[5].buffer_type = MYSQL_TYPE_LONG;
        result[5].buffer = &expireAt;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error("Failed to bind result: " + err);
        }

        std::vector<std::shared_ptr<WarrantyPolicy>> policies;
        while (mysql_stmt_fetch(stmt) == 0) {
            WarrantyPolicy::PolicyType policyTypeEnum = WarrantyPolicy::policyTypeFromString(std::string(policyType));
            auto policy = std::make_shared<WarrantyPolicy>(policyId, resultDeviceId, std::string(providerName),
                                                         policyTypeEnum, std::string(coverageDesc), expireAt);
            policies.push_back(policy);
        }

        mysql_stmt_close(stmt);
        return policies;
    }

    void deleteById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid warranty policy ID");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "DELETE FROM warranty_policies WHERE id = ?";
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

        mysql_stmt_close(stmt);
    }

    time_t getMaxExpireAtForDevice(int deviceId) {
        if (deviceId <= 0) {
            throw std::runtime_error("Invalid device ID");
        }

        auto conn = DBPool::getInstance().getConnection();
        MYSQL* mysqlConn = conn->getConn();

        std::string query = "SELECT MAX(expire_at) FROM warranty_policies WHERE device_id = ?";
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
        params[0].buffer = const_cast<int*>(&deviceId);

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

        MYSQL_BIND result[1] = {0};
        time_t maxExpireAt = 0;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &maxExpireAt;
        result[0].is_null = new my_bool();
        *result[0].is_null = 0;

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            delete result[0].is_null;
            throw std::runtime_error("Failed to bind result: " + err);
        }

        if (mysql_stmt_fetch(stmt) == 0 && *result[0].is_null == 0) {
            // 保留maxExpireAt
        } else {
            maxExpireAt = 0;
        }

        delete result[0].is_null;
        mysql_stmt_close(stmt);
        return maxExpireAt;
    }

private:
    WarrantyPolicyRepository(const WarrantyPolicyRepository&) = delete;
    WarrantyPolicyRepository& operator=(const WarrantyPolicyRepository&) = delete;

public:
    static WarrantyPolicyRepository& getInstance() {
        static WarrantyPolicyRepository instance;
        return instance;
    }
};
