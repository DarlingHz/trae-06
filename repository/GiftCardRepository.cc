#include "GiftCardRepository.h"
#include <utils/DatabasePool.h>
#include <utils/Logger.h>
#include <mysql/mysql.h>
#include <chrono>
#include <string>
#include <vector>

using namespace giftcard;

GiftCardRepository& GiftCardRepository::getInstance() {
    static GiftCardRepository instance;
    return instance;
}

bool GiftCardRepository::createGiftCard(const GiftCard& gift_card) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "INSERT INTO giftcards (card_no, user_id, template_id, balance, discount_rate, "
                          "valid_from, valid_to, status) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[8];
        memset(bind, 0, sizeof(bind));

        // card_no
        std::string card_no = gift_card.getCardNo();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)card_no.c_str();
        bind[0].buffer_length = card_no.length();

        // user_id
        uint64_t user_id = gift_card.getUserId();
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &user_id;

        // template_id
        uint64_t template_id = gift_card.getTemplateId();
        bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[2].buffer = &template_id;

        // balance
        double balance = gift_card.getBalance();
        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &balance;

        // discount_rate
        double discount_rate = gift_card.getDiscountRate();
        bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[4].buffer = &discount_rate;

        // valid_from
        auto valid_from = std::chrono::system_clock::to_time_t(gift_card.getValidFrom());
        MYSQL_TIME mysql_valid_from;
        tm* tm_valid_from = localtime(&valid_from);
        mysql_valid_from.year = tm_valid_from->tm_year + 1900;
        mysql_valid_from.month = tm_valid_from->tm_mon + 1;
        mysql_valid_from.day = tm_valid_from->tm_mday;
        mysql_valid_from.hour = tm_valid_from->tm_hour;
        mysql_valid_from.minute = tm_valid_from->tm_min;
        mysql_valid_from.second = tm_valid_from->tm_sec;
        bind[5].buffer_type = MYSQL_TYPE_DATETIME;
        bind[5].buffer = &mysql_valid_from;

        // valid_to
        auto valid_to = std::chrono::system_clock::to_time_t(gift_card.getValidTo());
        MYSQL_TIME mysql_valid_to;
        tm* tm_valid_to = localtime(&valid_to);
        mysql_valid_to.year = tm_valid_to->tm_year + 1900;
        mysql_valid_to.month = tm_valid_to->tm_mon + 1;
        mysql_valid_to.day = tm_valid_to->tm_mday;
        mysql_valid_to.hour = tm_valid_to->tm_hour;
        mysql_valid_to.minute = tm_valid_to->tm_min;
        mysql_valid_to.second = tm_valid_to->tm_sec;
        bind[6].buffer_type = MYSQL_TYPE_DATETIME;
        bind[6].buffer = &mysql_valid_to;

        // status
        std::string status = gift_card.getStatus();
        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = (char*)status.c_str();
        bind[7].buffer_length = status.length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute create gift card statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create gift card: %s", e.what());
        return false;
    }
}

bool GiftCardRepository::batchCreateGiftCards(const std::vector<GiftCard>& gift_cards) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "INSERT INTO giftcards (card_no, user_id, template_id, balance, discount_rate, "
                          "valid_from, valid_to, status) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0) {
            LOG_ERROR("Failed to prepare statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        MYSQL_BIND bind[8];
        memset(bind, 0, sizeof(bind));

        std::string card_no;
        uint64_t user_id;
        uint64_t template_id;
        double balance;
        double discount_rate;
        MYSQL_TIME mysql_valid_from;
        MYSQL_TIME mysql_valid_to;
        std::string status;

        // 绑定参数缓冲区
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)card_no.c_str();
        bind[0].buffer_length = card_no.length();

        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &user_id;

        bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[2].buffer = &template_id;

        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &balance;

        bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[4].buffer = &discount_rate;

        bind[5].buffer_type = MYSQL_TYPE_DATETIME;
        bind[5].buffer = &mysql_valid_from;

        bind[6].buffer_type = MYSQL_TYPE_DATETIME;
        bind[6].buffer = &mysql_valid_to;

        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = (char*)status.c_str();
        bind[7].buffer_length = status.length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        // 开始事务
        if (mysql_autocommit(conn.get(), 0) != 0) {
            LOG_ERROR("Failed to disable autocommit: %s", mysql_error(conn.get()));
            mysql_stmt_close(stmt);
            return false;
        }

        // 批量执行
        for (const auto& gift_card : gift_cards) {
            card_no = gift_card.getCardNo();
            bind[0].buffer = (char*)card_no.c_str();
            bind[0].buffer_length = card_no.length();

            user_id = gift_card.getUserId();
            template_id = gift_card.getTemplateId();
            balance = gift_card.getBalance();
            discount_rate = gift_card.getDiscountRate();

            // 处理valid_from
            auto valid_from = std::chrono::system_clock::to_time_t(gift_card.getValidFrom());
            tm* tm_valid_from = localtime(&valid_from);
            mysql_valid_from.year = tm_valid_from->tm_year + 1900;
            mysql_valid_from.month = tm_valid_from->tm_mon + 1;
            mysql_valid_from.day = tm_valid_from->tm_mday;
            mysql_valid_from.hour = tm_valid_from->tm_hour;
            mysql_valid_from.minute = tm_valid_from->tm_min;
            mysql_valid_from.second = tm_valid_from->tm_sec;

            // 处理valid_to
            auto valid_to = std::chrono::system_clock::to_time_t(gift_card.getValidTo());
            tm* tm_valid_to = localtime(&valid_to);
            mysql_valid_to.year = tm_valid_to->tm_year + 1900;
            mysql_valid_to.month = tm_valid_to->tm_mon + 1;
            mysql_valid_to.day = tm_valid_to->tm_mday;
            mysql_valid_to.hour = tm_valid_to->tm_hour;
            mysql_valid_to.minute = tm_valid_to->tm_min;
            mysql_valid_to.second = tm_valid_to->tm_sec;

            status = gift_card.getStatus();
            bind[7].buffer = (char*)status.c_str();
            bind[7].buffer_length = status.length();

            if (mysql_stmt_execute(stmt) != 0) {
                LOG_ERROR("Failed to execute batch create gift card statement: %s", mysql_stmt_error(stmt));
                mysql_rollback(conn.get());
                mysql_autocommit(conn.get(), 1);
                mysql_stmt_close(stmt);
                return false;
            }
        }

        // 提交事务
        if (mysql_commit(conn.get()) != 0) {
            LOG_ERROR("Failed to commit transaction: %s", mysql_error(conn.get()));
            mysql_rollback(conn.get());
            mysql_autocommit(conn.get(), 1);
            mysql_stmt_close(stmt);
            return false;
        }

        // 恢复自动提交
        if (mysql_autocommit(conn.get(), 1) != 0) {
            LOG_ERROR("Failed to enable autocommit: %s", mysql_error(conn.get()));
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to batch create gift cards: %s", e.what());
        return false;
    }
}

std::shared_ptr<GiftCard> GiftCardRepository::getGiftCardById(uint64_t card_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return nullptr;
        }

        const char* sql = "SELECT * FROM giftcards WHERE id = ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return nullptr;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift card by id statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_num_rows(stmt) == 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        auto gift_card = convertToGiftCard(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return gift_card;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card by id: %s", e.what());
        return nullptr;
    }
}

std::shared_ptr<GiftCard> GiftCardRepository::getGiftCardByCardNo(const std::string& card_no) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return nullptr;
        }

        const char* sql = "SELECT * FROM giftcards WHERE card_no = ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return nullptr;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)card_no.c_str();
        bind[0].buffer_length = card_no.length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift card by card no statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_num_rows(stmt) == 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        auto gift_card = convertToGiftCard(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return gift_card;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card by card no: %s", e.what());
        return nullptr;
    }
}

std::vector<std::shared_ptr<GiftCard>> GiftCardRepository::getGiftCardsByUserId(uint64_t user_id, const std::string& status) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return {};
        }

        std::string sql = "SELECT * FROM giftcards WHERE user_id = ?";
        std::vector<std::string> params;

        if (!status.empty()) {
            sql += " AND status = ?";
            params.push_back(status);
        }

        sql += " ORDER BY created_at DESC";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return {};
        }

        int param_count = params.size() + 1;
        MYSQL_BIND* bind = new MYSQL_BIND[param_count];
        memset(bind, 0, sizeof(MYSQL_BIND) * param_count);

        // 绑定user_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &user_id;

        // 绑定status参数
        for (int i = 0; i < params.size(); ++i) {
            bind[i + 1].buffer_type = MYSQL_TYPE_STRING;
            bind[i + 1].buffer = (char*)params[i].c_str();
            bind[i + 1].buffer_length = params[i].length();
        }

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift cards by user id statement: %s", mysql_stmt_error(stmt));
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            delete[] bind;
            mysql_stmt_close(stmt);
            return {};
        }

        auto gift_cards = convertToGiftCardList(result);

        mysql_free_result(result);
        delete[] bind;
        mysql_stmt_close(stmt);

        return gift_cards;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift cards by user id: %s", e.what());
        return {};
    }
}

bool GiftCardRepository::updateGiftCard(const GiftCard& gift_card) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "UPDATE giftcards SET card_no = ?, user_id = ?, template_id = ?, balance = ?, "
                          "discount_rate = ?, valid_from = ?, valid_to = ?, status = ? WHERE id = ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[10];
        memset(bind, 0, sizeof(bind));

        // card_no
        std::string card_no = gift_card.getCardNo();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)card_no.c_str();
        bind[0].buffer_length = card_no.length();

        // user_id
        uint64_t user_id = gift_card.getUserId();
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &user_id;

        // template_id
        uint64_t template_id = gift_card.getTemplateId();
        bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[2].buffer = &template_id;

        // balance
        double balance = gift_card.getBalance();
        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &balance;

        // discount_rate
        double discount_rate = gift_card.getDiscountRate();
        bind[4].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[4].buffer = &discount_rate;

        // valid_from
        auto valid_from = std::chrono::system_clock::to_time_t(gift_card.getValidFrom());
        MYSQL_TIME mysql_valid_from;
        tm* tm_valid_from = localtime(&valid_from);
        mysql_valid_from.year = tm_valid_from->tm_year + 1900;
        mysql_valid_from.month = tm_valid_from->tm_mon + 1;
        mysql_valid_from.day = tm_valid_from->tm_mday;
        mysql_valid_from.hour = tm_valid_from->tm_hour;
        mysql_valid_from.minute = tm_valid_from->tm_min;
        mysql_valid_from.second = tm_valid_from->tm_sec;
        bind[5].buffer_type = MYSQL_TYPE_DATETIME;
        bind[5].buffer = &mysql_valid_from;

        // valid_to
        auto valid_to = std::chrono::system_clock::to_time_t(gift_card.getValidTo());
        MYSQL_TIME mysql_valid_to;
        tm* tm_valid_to = localtime(&valid_to);
        mysql_valid_to.year = tm_valid_to->tm_year + 1900;
        mysql_valid_to.month = tm_valid_to->tm_mon + 1;
        mysql_valid_to.day = tm_valid_to->tm_mday;
        mysql_valid_to.hour = tm_valid_to->tm_hour;
        mysql_valid_to.minute = tm_valid_to->tm_min;
        mysql_valid_to.second = tm_valid_to->tm_sec;
        bind[6].buffer_type = MYSQL_TYPE_DATETIME;
        bind[6].buffer = &mysql_valid_to;

        // status
        std::string status = gift_card.getStatus();
        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = (char*)status.c_str();
        bind[7].buffer_length = status.length();

        // id
        uint64_t id = gift_card.getId();
        bind[8].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[8].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute update gift card statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No gift card found with id: %llu", gift_card.getId());
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update gift card: %s", e.what());
        return false;
    }
}

bool GiftCardRepository::deductGiftCardBalance(uint64_t card_id, double amount) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "UPDATE giftcards SET balance = balance - ? WHERE id = ? AND balance >= ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        // amount
        bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[0].buffer = &amount;

        // card_id
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &card_id;

        // amount (for balance check)
        bind[2].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[2].buffer = &amount;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute deduct gift card balance statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No gift card found with id: %llu or insufficient balance", card_id);
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to deduct gift card balance: %s", e.what());
        return false;
    }
}

bool GiftCardRepository::freezeGiftCard(uint64_t card_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "UPDATE giftcards SET status = 'frozen' WHERE id = ? AND status != 'frozen'";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute freeze gift card statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No gift card found with id: %llu or already frozen", card_id);
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to freeze gift card: %s", e.what());
        return false;
    }
}

bool GiftCardRepository::unfreezeGiftCard(uint64_t card_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "UPDATE giftcards SET status = 'available' WHERE id = ? AND status = 'frozen'";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute unfreeze gift card statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No gift card found with id: %llu or not frozen", card_id);
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to unfreeze gift card: %s", e.what());
        return false;
    }
}

bool GiftCardRepository::createGiftCardLock(const GiftCardLock& lock) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "INSERT INTO giftcard_locks (card_id, user_id, order_id, lock_amount, lock_ttl, status) "
                          "VALUES (?, ?, ?, ?, ?, ?)";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[6];
        memset(bind, 0, sizeof(bind));

        // card_id
        uint64_t card_id = lock.getCardId();
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        // user_id
        uint64_t user_id = lock.getUserId();
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &user_id;

        // order_id
        std::string order_id = lock.getOrderId();
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)order_id.c_str();
        bind[2].buffer_length = order_id.length();

        // lock_amount
        double lock_amount = lock.getLockAmount();
        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &lock_amount;

        // lock_ttl
        uint32_t lock_ttl = lock.getLockTtl();
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &lock_ttl;

        // status
        std::string status = lock.getStatus();
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)status.c_str();
        bind[5].buffer_length = status.length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute create gift card lock statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create gift card lock: %s", e.what());
        return false;
    }
}

std::shared_ptr<GiftCardLock> GiftCardRepository::getGiftCardLockById(uint64_t lock_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return nullptr;
        }

        const char* sql = "SELECT * FROM giftcard_locks WHERE id = ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return nullptr;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &lock_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift card lock by id statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_num_rows(stmt) == 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        auto lock = convertToGiftCardLock(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return lock;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card lock by id: %s", e.what());
        return nullptr;
    }
}

std::shared_ptr<GiftCardLock> GiftCardRepository::getGiftCardLockByCardIdAndOrderId(uint64_t card_id, const std::string& order_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return nullptr;
        }

        const char* sql = "SELECT * FROM giftcard_locks WHERE card_id = ? AND order_id = ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return nullptr;
        }

        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // card_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        // order_id
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)order_id.c_str();
        bind[1].buffer_length = order_id.length();

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift card lock by card id and order id statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_num_rows(stmt) == 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        auto lock = convertToGiftCardLock(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return lock;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card lock by card id and order id: %s", e.what());
        return nullptr;
    }
}

std::vector<std::shared_ptr<GiftCardLock>> GiftCardRepository::getActiveGiftCardLocks(uint64_t card_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return {};
        }

        const char* sql = "SELECT * FROM giftcard_locks WHERE card_id = ? AND status = 'active' AND created_at + INTERVAL lock_ttl SECOND > NOW()";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return {};
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get active gift card locks statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return {};
        }

        auto locks = convertToGiftCardLockList(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return locks;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get active gift card locks: %s", e.what());
        return {};
    }
}

bool GiftCardRepository::updateGiftCardLock(const GiftCardLock& lock) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "UPDATE giftcard_locks SET card_id = ?, user_id = ?, order_id = ?, lock_amount = ?, "
                          "lock_ttl = ?, status = ? WHERE id = ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[8];
        memset(bind, 0, sizeof(bind));

        // card_id
        uint64_t card_id = lock.getCardId();
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        // user_id
        uint64_t user_id = lock.getUserId();
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &user_id;

        // order_id
        std::string order_id = lock.getOrderId();
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)order_id.c_str();
        bind[2].buffer_length = order_id.length();

        // lock_amount
        double lock_amount = lock.getLockAmount();
        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &lock_amount;

        // lock_ttl
        uint32_t lock_ttl = lock.getLockTtl();
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &lock_ttl;

        // status
        std::string status = lock.getStatus();
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)status.c_str();
        bind[5].buffer_length = status.length();

        // id
        uint64_t id = lock.getId();
        bind[6].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[6].buffer = &id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute update gift card lock statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No gift card lock found with id: %llu", lock.getId());
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to update gift card lock: %s", e.what());
        return false;
    }
}

bool GiftCardRepository::releaseGiftCardLock(uint64_t lock_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "UPDATE giftcard_locks SET status = 'released' WHERE id = ? AND status = 'active'";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &lock_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute release gift card lock statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_affected_rows(stmt) == 0) {
            LOG_WARN("No active gift card lock found with id: %llu", lock_id);
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to release gift card lock: %s", e.what());
        return false;
    }
}

bool GiftCardRepository::createGiftCardConsumption(const GiftCardConsumption& consumption) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return false;
        }

        const char* sql = "INSERT INTO giftcard_consumptions (card_id, user_id, order_id, consume_amount) "
                          "VALUES (?, ?, ?, ?)";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return false;
        }

        MYSQL_BIND bind[4];
        memset(bind, 0, sizeof(bind));

        // card_id
        uint64_t card_id = consumption.getCardId();
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        // user_id
        uint64_t user_id = consumption.getUserId();
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &user_id;

        // order_id
        std::string order_id = consumption.getOrderId();
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)order_id.c_str();
        bind[2].buffer_length = order_id.length();

        // consume_amount
        double consume_amount = consumption.getConsumeAmount();
        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &consume_amount;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute create gift card consumption statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return false;
        }

        mysql_stmt_close(stmt);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create gift card consumption: %s", e.what());
        return false;
    }
}

std::shared_ptr<GiftCardConsumption> GiftCardRepository::getGiftCardConsumptionById(uint64_t consumption_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return nullptr;
        }

        const char* sql = "SELECT * FROM giftcard_consumptions WHERE id = ?";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return nullptr;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &consumption_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift card consumption by id statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        if (mysql_stmt_num_rows(stmt) == 0) {
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return nullptr;
        }

        auto consumption = convertToGiftCardConsumption(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return consumption;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card consumption by id: %s", e.what());
        return nullptr;
    }
}

std::vector<std::shared_ptr<GiftCardConsumption>> GiftCardRepository::getGiftCardConsumptionsByCardId(uint64_t card_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return {};
        }

        const char* sql = "SELECT * FROM giftcard_consumptions WHERE card_id = ? ORDER BY consume_time DESC";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return {};
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &card_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift card consumptions by card id statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return {};
        }

        auto consumptions = convertToGiftCardConsumptionList(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return consumptions;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card consumptions by card id: %s", e.what());
        return {};
    }
}

std::vector<std::shared_ptr<GiftCardConsumption>> GiftCardRepository::getGiftCardConsumptionsByUserId(uint64_t user_id) {
    try {
        auto conn = DatabasePool::getInstance().getConnection();
        if (!conn) {
            LOG_ERROR("Failed to get database connection");
            return {};
        }

        const char* sql = "SELECT * FROM giftcard_consumptions WHERE user_id = ? ORDER BY consume_time DESC";

        MYSQL_STMT* stmt = mysql_stmt_init(conn.get());
        if (!stmt) {
            LOG_ERROR("Failed to initialize MySQL statement: %s", mysql_error(conn.get()));
            return {};
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &user_id;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            LOG_ERROR("Failed to bind parameters: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_execute(stmt) != 0) {
            LOG_ERROR("Failed to execute get gift card consumptions by user id statement: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        MYSQL_RES* result = mysql_stmt_result_metadata(stmt);
        if (!result) {
            LOG_ERROR("Failed to get result metadata: %s", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            return {};
        }

        if (mysql_stmt_store_result(stmt) != 0) {
            LOG_ERROR("Failed to store result: %s", mysql_stmt_error(stmt));
            mysql_free_result(result);
            mysql_stmt_close(stmt);
            return {};
        }

        auto consumptions = convertToGiftCardConsumptionList(result);

        mysql_free_result(result);
        mysql_stmt_close(stmt);

        return consumptions;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gift card consumptions by user id: %s", e.what());
        return {};
    }
}

std::shared_ptr<GiftCard> GiftCardRepository::convertToGiftCard(MYSQL_RES* result) {
    if (!result) {
        return nullptr;
    }

    auto gift_card = std::make_shared<GiftCard>();

    int field_count = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    char** buffer = new char*[field_count];
    unsigned long* lengths = new unsigned long[field_count];
    memset(buffer, 0, sizeof(char*) * field_count);
    memset(lengths, 0, sizeof(unsigned long) * field_count);

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        delete[] buffer;
        delete[] lengths;
        return nullptr;
    }

    lengths = mysql_fetch_lengths(result);

    for (int i = 0; i < field_count; ++i) {
        if (!row[i]) {
            continue;
        }

        std::string field_name(fields[i].name);
        std::string field_value(row[i], lengths[i]);

        if (field_name == "id") {
            gift_card->setId(std::stoull(field_value));
        } else if (field_name == "card_no") {
            gift_card->setCardNo(field_value);
        } else if (field_name == "user_id") {
            gift_card->setUserId(std::stoull(field_value));
        } else if (field_name == "template_id") {
            gift_card->setTemplateId(std::stoull(field_value));
        } else if (field_name == "balance") {
            gift_card->setBalance(std::stod(field_value));
        } else if (field_name == "discount_rate") {
            gift_card->setDiscountRate(std::stod(field_value));
        } else if (field_name == "valid_from") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto valid_from = std::chrono::system_clock::from_time_t(mktime(&tm));
            gift_card->setValidFrom(valid_from);
        } else if (field_name == "valid_to") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto valid_to = std::chrono::system_clock::from_time_t(mktime(&tm));
            gift_card->setValidTo(valid_to);
        } else if (field_name == "status") {
            gift_card->setStatus(field_value);
        } else if (field_name == "created_at") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
            gift_card->setCreatedAt(created_at);
        } else if (field_name == "updated_at") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
            gift_card->setUpdatedAt(updated_at);
        }
    }

    delete[] buffer;
    delete[] lengths;

    return gift_card;
}

std::vector<std::shared_ptr<GiftCard>> GiftCardRepository::convertToGiftCardList(MYSQL_RES* result) {
    std::vector<std::shared_ptr<GiftCard>> gift_cards;

    if (!result) {
        return gift_cards;
    }

    int field_count = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        auto gift_card = std::make_shared<GiftCard>();

        unsigned long* lengths = mysql_fetch_lengths(result);

        for (int i = 0; i < field_count; ++i) {
            if (!row[i]) {
                continue;
            }

            std::string field_name(fields[i].name);
            std::string field_value(row[i], lengths[i]);

            if (field_name == "id") {
                gift_card->setId(std::stoull(field_value));
            } else if (field_name == "card_no") {
                gift_card->setCardNo(field_value);
            } else if (field_name == "user_id") {
                gift_card->setUserId(std::stoull(field_value));
            } else if (field_name == "template_id") {
                gift_card->setTemplateId(std::stoull(field_value));
            } else if (field_name == "balance") {
                gift_card->setBalance(std::stod(field_value));
            } else if (field_name == "discount_rate") {
                gift_card->setDiscountRate(std::stod(field_value));
            } else if (field_name == "valid_from") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto valid_from = std::chrono::system_clock::from_time_t(mktime(&tm));
                gift_card->setValidFrom(valid_from);
            } else if (field_name == "valid_to") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto valid_to = std::chrono::system_clock::from_time_t(mktime(&tm));
                gift_card->setValidTo(valid_to);
            } else if (field_name == "status") {
                gift_card->setStatus(field_value);
            } else if (field_name == "created_at") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
                gift_card->setCreatedAt(created_at);
            } else if (field_name == "updated_at") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
                gift_card->setUpdatedAt(updated_at);
            }
        }

        gift_cards.push_back(gift_card);
    }

    return gift_cards;
}

std::shared_ptr<GiftCardLock> GiftCardRepository::convertToGiftCardLock(MYSQL_RES* result) {
    if (!result) {
        return nullptr;
    }

    auto lock = std::make_shared<GiftCardLock>();

    int field_count = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    char** buffer = new char*[field_count];
    unsigned long* lengths = new unsigned long[field_count];
    memset(buffer, 0, sizeof(char*) * field_count);
    memset(lengths, 0, sizeof(unsigned long) * field_count);

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        delete[] buffer;
        delete[] lengths;
        return nullptr;
    }

    lengths = mysql_fetch_lengths(result);

    for (int i = 0; i < field_count; ++i) {
        if (!row[i]) {
            continue;
        }

        std::string field_name(fields[i].name);
        std::string field_value(row[i], lengths[i]);

        if (field_name == "id") {
            lock->setId(std::stoull(field_value));
        } else if (field_name == "card_id") {
            lock->setCardId(std::stoull(field_value));
        } else if (field_name == "user_id") {
            lock->setUserId(std::stoull(field_value));
        } else if (field_name == "order_id") {
            lock->setOrderId(field_value);
        } else if (field_name == "lock_amount") {
            lock->setLockAmount(std::stod(field_value));
        } else if (field_name == "lock_ttl") {
            lock->setLockTtl(std::stoul(field_value));
        } else if (field_name == "status") {
            lock->setStatus(field_value);
        } else if (field_name == "created_at") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
            lock->setCreatedAt(created_at);
        } else if (field_name == "updated_at") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
            lock->setUpdatedAt(updated_at);
        }
    }

    delete[] buffer;
    delete[] lengths;

    return lock;
}

std::vector<std::shared_ptr<GiftCardLock>> GiftCardRepository::convertToGiftCardLockList(MYSQL_RES* result) {
    std::vector<std::shared_ptr<GiftCardLock>> locks;

    if (!result) {
        return locks;
    }

    int field_count = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        auto lock = std::make_shared<GiftCardLock>();

        unsigned long* lengths = mysql_fetch_lengths(result);

        for (int i = 0; i < field_count; ++i) {
            if (!row[i]) {
                continue;
            }

            std::string field_name(fields[i].name);
            std::string field_value(row[i], lengths[i]);

            if (field_name == "id") {
                lock->setId(std::stoull(field_value));
            } else if (field_name == "card_id") {
                lock->setCardId(std::stoull(field_value));
            } else if (field_name == "user_id") {
                lock->setUserId(std::stoull(field_value));
            } else if (field_name == "order_id") {
                lock->setOrderId(field_value);
            } else if (field_name == "lock_amount") {
                lock->setLockAmount(std::stod(field_value));
            } else if (field_name == "lock_ttl") {
                lock->setLockTtl(std::stoul(field_value));
            } else if (field_name == "status") {
                lock->setStatus(field_value);
            } else if (field_name == "created_at") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto created_at = std::chrono::system_clock::from_time_t(mktime(&tm));
                lock->setCreatedAt(created_at);
            } else if (field_name == "updated_at") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto updated_at = std::chrono::system_clock::from_time_t(mktime(&tm));
                lock->setUpdatedAt(updated_at);
            }
        }

        locks.push_back(lock);
    }

    return locks;
}

std::shared_ptr<GiftCardConsumption> GiftCardRepository::convertToGiftCardConsumption(MYSQL_RES* result) {
    if (!result) {
        return nullptr;
    }

    auto consumption = std::make_shared<GiftCardConsumption>();

    int field_count = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    char** buffer = new char*[field_count];
    unsigned long* lengths = new unsigned long[field_count];
    memset(buffer, 0, sizeof(char*) * field_count);
    memset(lengths, 0, sizeof(unsigned long) * field_count);

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        delete[] buffer;
        delete[] lengths;
        return nullptr;
    }

    lengths = mysql_fetch_lengths(result);

    for (int i = 0; i < field_count; ++i) {
        if (!row[i]) {
            continue;
        }

        std::string field_name(fields[i].name);
        std::string field_value(row[i], lengths[i]);

        if (field_name == "id") {
            consumption->setId(std::stoull(field_value));
        } else if (field_name == "card_id") {
            consumption->setCardId(std::stoull(field_value));
        } else if (field_name == "user_id") {
            consumption->setUserId(std::stoull(field_value));
        } else if (field_name == "order_id") {
            consumption->setOrderId(field_value);
        } else if (field_name == "consume_amount") {
            consumption->setConsumeAmount(std::stod(field_value));
        } else if (field_name == "consume_time") {
            std::tm tm = {};
            strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
            auto consume_time = std::chrono::system_clock::from_time_t(mktime(&tm));
            consumption->setConsumeTime(consume_time);
        }
    }

    delete[] buffer;
    delete[] lengths;

    return consumption;
}

std::vector<std::shared_ptr<GiftCardConsumption>> GiftCardRepository::convertToGiftCardConsumptionList(MYSQL_RES* result) {
    std::vector<std::shared_ptr<GiftCardConsumption>> consumptions;

    if (!result) {
        return consumptions;
    }

    int field_count = mysql_num_fields(result);
    MYSQL_FIELD* fields = mysql_fetch_fields(result);

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result)) != nullptr) {
        auto consumption = std::make_shared<GiftCardConsumption>();

        unsigned long* lengths = mysql_fetch_lengths(result);

        for (int i = 0; i < field_count; ++i) {
            if (!row[i]) {
                continue;
            }

            std::string field_name(fields[i].name);
            std::string field_value(row[i], lengths[i]);

            if (field_name == "id") {
                consumption->setId(std::stoull(field_value));
            } else if (field_name == "card_id") {
                consumption->setCardId(std::stoull(field_value));
            } else if (field_name == "user_id") {
                consumption->setUserId(std::stoull(field_value));
            } else if (field_name == "order_id") {
                consumption->setOrderId(field_value);
            } else if (field_name == "consume_amount") {
                consumption->setConsumeAmount(std::stod(field_value));
            } else if (field_name == "consume_time") {
                std::tm tm = {};
                strptime(field_value.c_str(), "%Y-%m-%d %H:%M:%S", &tm);
                auto consume_time = std::chrono::system_clock::from_time_t(mktime(&tm));
                consumption->setConsumeTime(consume_time);
            }
        }

        consumptions.push_back(consumption);
    }

    return consumptions;
}

} // namespace giftcard
