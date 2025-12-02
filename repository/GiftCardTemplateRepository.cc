#include "GiftCardTemplateRepository.h"
#include <utils/DatabasePool.h>
#include <utils/Logger.h>
#include <mysql/mysql.h>
#include <chrono>
#include <string>

using namespace giftcard;

bool GiftCardTemplateRepository::createTemplate(const GiftCardTemplate& template_) {
    try {
        // 获取数据库连接
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        // 准备SQL语句
        const char* sql = "INSERT INTO giftcard_templates (name, type, face_value, min_order_amount, "
                          "total_stock, per_user_limit, valid_from, valid_to) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

        // 创建预处理语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[8];
        memset(bind, 0, sizeof(bind));

        // name
        std::string name = template_.getName();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)name.c_str();
        bind[0].buffer_length = name.length();

        // type
        std::string type = (template_.getType() == TemplateType::AMOUNT) ? "amount" : "discount";
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)type.c_str();
        bind[1].buffer_length = type.length();

        // face_value
        double face_value = template_.getFaceValue();
        bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[2].buffer = &face_value;

        // min_order_amount
        double min_order_amount = template_.getMinOrderAmount();
        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &min_order_amount;

        // total_stock
        uint32_t total_stock = template_.getTotalStock();
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &total_stock;

        // per_user_limit
        uint32_t per_user_limit = template_.getPerUserLimit();
        bind[5].buffer_type = MYSQL_TYPE_LONG;
        bind[5].buffer = &per_user_limit;

        // valid_from
        auto valid_from = std::chrono::system_clock::to_time_t(template_.getValidFrom());
        MYSQL_TIME mysql_valid_from;
        tm* tm_valid_from = localtime(&valid_from);
        mysql_valid_from.year = tm_valid_from->tm_year + 1900;
        mysql_valid_from.month = tm_valid_from->tm_mon + 1;
        mysql_valid_from.day = tm_valid_from->tm_mday;
        mysql_valid_from.hour = tm_valid_from->tm_hour;
        mysql_valid_from.minute = tm_valid_from->tm_min;
        mysql_valid_from.second = tm_valid_from->tm_sec;
        bind[6].buffer_type = MYSQL_TYPE_DATETIME;
        bind[6].buffer = &mysql_valid_from;

        // valid_to
        auto valid_to = std::chrono::system_clock::to_time_t(template_.getValidTo());
        MYSQL_TIME mysql_valid_to;
        tm* tm_valid_to = localtime(&valid_to);
        mysql_valid_to.year = tm_valid_to->tm_year + 1900;
        mysql_valid_to.month = tm_valid_to->tm_mon + 1;
        mysql_valid_to.day = tm_valid_to->tm_mday;
        mysql_valid_to.hour = tm_valid_to->tm_hour;
        mysql_valid_to.minute = tm_valid_to->tm_min;
        mysql_valid_to.second = tm_valid_to->tm_sec;
        bind[7].buffer_type = MYSQL_TYPE_DATETIME;
        bind[7].buffer = &mysql_valid_to;

        // 绑定参数到预处理语句
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 执行预处理语句
        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute create template statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 关闭预处理语句
        mysql_stmt_close(stmt);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create gift card template: %s", e.what());
        return false;
    }
}

std::shared_ptr<GiftCardTemplate> GiftCardTemplateRepository::getTemplateById(uint64_t template_id) {
    try {
        // 获取数据库连接
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return nullptr;
        }

        // 准备SQL语句
        const char* sql = "SELECT * FROM giftcard_templates WHERE id = ?";

        // 创建预处理语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return nullptr;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        // template_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &template_id;

        // 绑定参数到预处理语句
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        // 执行预处理语句
        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get template by id statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        // 获取结果集
        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        // 存储结果
        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        // 获取行数
        if (mysql_stmt_num_rows(stmt) == 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        // 转换结果集为GiftCardTemplate对象
        auto template_ = convertToTemplate(result);

        // 清理资源
        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return template_;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card template by id: %s", e.what());
        return nullptr;
    }
}

std::vector<std::shared_ptr<GiftCardTemplate>> GiftCardTemplateRepository::getTemplates(
    const std::string& name,
    const std::string& status,
    int page,
    int page_size
) {
    try {
        // 获取数据库连接
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return {};
        }

        // 构建SQL语句
        std::string sql = "SELECT * FROM giftcard_templates WHERE 1=1";
        std::vector<std::string> params;

        if (!name.empty()) {
            sql += " AND name LIKE ?";
            params.push_back("%" + name + "%");
        }

        if (!status.empty()) {
            sql += " AND status = ?";
            params.push_back(status);
        }

        // 添加分页
        sql += " ORDER BY created_at DESC LIMIT ? OFFSET ?";
        int offset = (page - 1) * page_size;

        // 创建预处理语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return {};
        }

        // 绑定参数
        int param_count = params.size() + 2;
        MYSQL_BIND* bind = new MYSQL_BIND[param_count];
        memset(bind, 0, sizeof(MYSQL_BIND) * param_count);

        // 绑定搜索参数
        for (int i = 0; i < params.size(); ++i) {
            bind[i].buffer_type = MYSQL_TYPE_STRING;
            bind[i].buffer = (char*)params[i].c_str();
            bind[i].buffer_length = params[i].length();
        }

        // 绑定分页参数
        int page_size_param = page_size;
        bind[params.size()].buffer_type = MYSQL_TYPE_LONG;
        bind[params.size()].buffer = &page_size_param;

        int offset_param = offset;
        bind[params.size() + 1].buffer_type = MYSQL_TYPE_LONG;
        bind[params.size() + 1].buffer = &offset_param;

        // 绑定参数到预处理语句
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        // 执行预处理语句
        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get templates statement: %s", mysql_stmt_error(stmt));
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        // 获取结果集
        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        // 存储结果
        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        // 转换结果集为GiftCardTemplate对象列表
        auto templates = convertToTemplateList(result);

        // 清理资源
        mysql_free_result(result);
        delete[] bind;
        mysql_stmt_close(stmt);

        return templates;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card templates: %s", e.what());
        return {};
    }
}

bool GiftCardTemplateRepository::updateTemplate(const GiftCardTemplate& template_) {
    try {
        // 获取数据库连接
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        // 准备SQL语句
        const char* sql = "UPDATE giftcard_templates SET name = ?, type = ?, face_value = ?, "
                          "min_order_amount = ?, total_stock = ?, per_user_limit = ?, "
                          "valid_from = ?, valid_to = ? WHERE id = ?";

        // 创建预处理语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[10];
        memset(bind, 0, sizeof(bind));

        // name
        std::string name = template_.getName();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)name.c_str();
        bind[0].buffer_length = name.length();

        // type
        std::string type = (template_.getType() == TemplateType::AMOUNT) ? "amount" : "discount";
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)type.c_str();
        bind[1].buffer_length = type.length();

        // face_value
        double face_value = template_.getFaceValue();
        bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[2].buffer = &face_value;

        // min_order_amount
        double min_order_amount = template_.getMinOrderAmount();
        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &min_order_amount;

        // total_stock
        uint32_t total_stock = template_.getTotalStock();
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &total_stock;

        // per_user_limit
        uint32_t per_user_limit = template_.getPerUserLimit();
        bind[5].buffer_type = MYSQL_TYPE_LONG;
        bind[5].buffer = &per_user_limit;

        // valid_from
        auto valid_from = std::chrono::system_clock::to_time_t(template_.getValidFrom());
        MYSQL_TIME mysql_valid_from;
        tm* tm_valid_from = localtime(&valid_from);
        mysql_valid_from.year = tm_valid_from->tm_year + 1900;
        mysql_valid_from.month = tm_valid_from->tm_mon + 1;
        mysql_valid_from.day = tm_valid_from->tm_mday;
        mysql_valid_from.hour = tm_valid_from->tm_hour;
        mysql_valid_from.minute = tm_valid_from->tm_min;
        mysql_valid_from.second = tm_valid_from->tm_sec;
        bind[6].buffer_type = MYSQL_TYPE_DATETIME;
        bind[6].buffer = &mysql_valid_from;

        // valid_to
        auto valid_to = std::chrono::system_clock::to_time_t(template_.getValidTo());
        MYSQL_TIME mysql_valid_to;
        tm* tm_valid_to = localtime(&valid_to);
        mysql_valid_to.year = tm_valid_to->tm_year + 1900;
        mysql_valid_to.month = tm_valid_to->tm_mon + 1;
        mysql_valid_to.day = tm_valid_to->tm_mday;
        mysql_valid_to.hour = tm_valid_to->tm_hour;
        mysql_valid_to.minute = tm_valid_to->tm_min;
        mysql_valid_to.second = tm_valid_to->tm_sec;
        bind[7].buffer_type = MYSQL_TYPE_DATETIME;
        bind[7].buffer = &mysql_valid_to;

        // id
        uint64_t id = template_.getId();
        bind[8].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[8].buffer = &id;

        // 绑定参数到预处理语句
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 执行预处理语句
        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute update template statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 检查受影响的行数
        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No template found with id: %llu", template_.getId());
            mysql_stmt_close(stmt);
            return false;
        }

        // 关闭预处理语句
        mysql_stmt_close(stmt);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update gift card template: %s", e.what());
        return false;
    }
}

bool GiftCardTemplateRepository::closeTemplate(uint64_t template_id) {
    try {
        // 获取数据库连接
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        // 准备SQL语句
        const char* sql = "UPDATE giftcard_templates SET status = 'closed' WHERE id = ? AND status = 'active'";

        // 创建预处理语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        // template_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &template_id;

        // 绑定参数到预处理语句
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 执行预处理语句
        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute close template statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 检查受影响的行数
        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No active template found with id: %llu", template_id);
            mysql_stmt_close(stmt);
            return false;
        }

        // 关闭预处理语句
        mysql_stmt_close(stmt);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to close gift card template: %s", e.what());
        return false;
    }
}

bool GiftCardTemplateRepository::updateTemplateIssuedCount(uint64_t template_id, int increment) {
    try {
        // 获取数据库连接
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        // 准备SQL语句
        const char* sql = "UPDATE giftcard_templates SET issued_count = issued_count + ? WHERE id = ?";

        // 创建预处理语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // increment
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &increment;

        // template_id
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &template_id;

        // 绑定参数到预处理语句
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 执行预处理语句
        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute update template issued count statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 检查受影响的行数
        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No template found with id: %llu", template_id);
            mysql_stmt_close(stmt);
            return false;
        }

        // 关闭预处理语句
        mysql_stmt_close(stmt);

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update gift card template issued count: %s", e.what());
        return false;
    }
}

std::shared_ptr<GiftCardTemplate> GiftCardTemplateRepository::convertToTemplate(MYSQL_RES* result) {
    if (!result) {
        return nullptr;
    }

    auto template_ = std::make_shared<GiftCardTemplate>();

    // 获取字段数量
    int field_count = mysql_num_fields(result);

    // 获取字段信息
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    // 创建缓冲区存储字段值
    char** buffer = new char*[field_count];
    unsigned long* lengths = new unsigned long[field_count];
    memset(buffer, 0, sizeof(char*) * field_count);
    memset(lengths, 0, sizeof(unsigned long) * field_count);

    // 获取一行结果
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        delete[] buffer;
        delete[] lengths;
        return nullptr;
    }

    // 获取字段长度
    lengths = mysql_fetch_lengths(result);

    // 填充GiftCardTemplate对象
    for (int i = 0; i < field_count; ++i) {
        if (!row[i]) {
            continue;
        }

        std::string field_name(fields[i].name);
        std::string field_value(row[i], lengths[i]);

        if (field_name == "id") {
            template_->setId(std::stoull(field_value));
        } else if (field_name == "name") {
            template_->setName(field_value);
        } else if (field_name == "type") {
            template_->setType(field_value == "amount" ? TemplateType::AMOUNT : TemplateType::DISCOUNT);
        } else if (field_name == "face_value") {
            template_->setFaceValue(std::stod(field_value));
        } else if (field_name == "min_order_amount") {
            template_->setMinOrderAmount(std::stod(field_value));
        } else if (field_name == "total_stock") {
            template_->setTotalStock(std::stoul(field_value));
        } else if (field_name == "issued_count") {
            template_->setIssuedCount(std::stoul(field_value));
        } else if (field_name == "per_user_limit") {
            template_->setPerUserLimit(std::stoul(field_value));
        } else if (field_name == "valid_from") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto valid_from = std::chrono::system_clock::from_time_t(mktime(&tm));
            template_->setValidFrom(valid_from);
        } else if (field_name == "valid_to") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto valid_to = std::chrono::system_clock::from_time_t(mktime(&tm));
            template_->setValidTo(valid_to);
        } else if (field_name == "status") {
            template_->setStatus(field_value == "active" ? TemplateStatus::ACTIVE : TemplateStatus::CLOSED);
        } else if (field_name == "created_at") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
            template_->setCreatedAt(created_at);
        } else if (field_name == "updated_at") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
            template_->setUpdatedAt(updated_at);
        }
    }

    // 清理资源
    delete[] buffer;
    delete[] lengths;

    return template_;
}

std::vector<std::shared_ptr<GiftCardTemplate>> GiftCardTemplateRepository::convertToTemplateList(MYSQL_RES* result) {
    std::vector<std::shared_ptr<GiftCardTemplate>> templates;

    if (!result) {
        return templates;
    }

    // 获取字段数量
    int field_count = mysql_num_fields(result);

    // 获取字段信息
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    // 获取所有行
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        auto template_ = std::make_shared<GiftCardTemplate>();

        // 获取字段长度
        unsigned long* lengths = mysql_fetch_lengths(result);

        // 填充GiftCardTemplate对象
        for (int i = 0; i < field_count; ++i) {
            if (!row[i]) {
                continue;
            }

            std::string field_name(fields[i].name);
            std::string field_value(row[i], lengths[i]);

            if (field_name == "id") {
                template_->setId(std::stoull(field_value));
            } else if (field_name == "name") {
                template_->setName(field_value);
            } else if (field_name == "type") {
                template_->setType(field_value == "amount" ? TemplateType::AMOUNT : TemplateType::DISCOUNT);
            } else if (field_name == "face_value") {
                template_->setFaceValue(std::stod(field_value));
            } else if (field_name == "min_order_amount") {
                template_->setMinOrderAmount(std::stod(field_value));
            } else if (field_name == "total_stock") {
                template_->setTotalStock(std::stoul(field_value));
            } else if (field_name == "issued_count") {
                template_->setIssuedCount(std::stoul(field_value));
            } else if (field_name == "per_user_limit") {
                template_->setPerUserLimit(std::stoul(field_value));
            } else if (field_name == "valid_from") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto valid_from = std::chrono::system_clock::from_time_t(mktime(&tm));
                template_->setValidFrom(valid_from);
            } else if (field_name == "valid_to") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto valid_to = std::chrono::system_clock::from_time_t(mktime(&tm));
                template_->setValidTo(valid_to);
            } else if (field_name == "status") {
                template_->setStatus(field_value == "active" ? TemplateStatus::ACTIVE : TemplateStatus::CLOSED);
            } else if (field_name == "created_at") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
                template_->setCreatedAt(created_at);
            } else if (field_name == "updated_at") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
                template_->setUpdatedAt(updated_at);
            }
        }

        templates.push_back(template_);
    }

    return templates;
}

} // namespace giftcard
