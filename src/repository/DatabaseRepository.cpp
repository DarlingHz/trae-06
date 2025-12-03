#include <repository/DatabaseRepository.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace event_signup_service::repository {

DatabaseRepository::DatabaseRepository(const std::string& db_path) {
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("无法打开数据库: " + std::string(sqlite3_errmsg(db_)));
    }
}

DatabaseRepository::~DatabaseRepository() {
    if (db_ != nullptr) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

DatabaseRepository::DatabaseRepository(DatabaseRepository&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

DatabaseRepository& DatabaseRepository::operator=(DatabaseRepository&& other) noexcept {
    if (this != &other) {
        if (db_ != nullptr) {
            sqlite3_close(db_);
        }
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

int64_t DatabaseRepository::get_last_insert_id() {
    return sqlite3_last_insert_rowid(db_);
}

std::string DatabaseRepository::escape_string(const std::string& str) const {
    char* escaped = sqlite3_mprintf("%q", str.c_str());
    if (escaped == nullptr) {
        throw std::runtime_error("内存分配失败: 转义字符串时出错");
    }
    std::string result(escaped);
    sqlite3_free(escaped);
    return result;
}

std::chrono::system_clock::time_point DatabaseRepository::parse_datetime(const std::string& datetime_str) const {
    // SQLite的datetime格式: YYYY-MM-DD HH:MM:SS
    std::tm tm = {};
    std::istringstream ss(datetime_str);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    if (ss.fail()) {
        throw std::runtime_error("解析时间失败: " + datetime_str);
    }
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string DatabaseRepository::format_datetime(const std::chrono::system_clock::time_point& time) const {
    std::time_t tt = std::chrono::system_clock::to_time_t(time);
    std::tm tm = *std::gmtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void DatabaseRepository::begin_transaction() {
    if (in_transaction_) {
        throw std::runtime_error("已经在事务中");
    }
    const char* sql = "BEGIN TRANSACTION;";
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("开始事务失败: " + err_str);
    }
    in_transaction_ = true;
}

void DatabaseRepository::commit_transaction() {
    if (!in_transaction_) {
        throw std::runtime_error("不在事务中");
    }
    const char* sql = "COMMIT TRANSACTION;";
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("提交事务失败: " + err_str);
    }
    in_transaction_ = false;
}

void DatabaseRepository::rollback_transaction() {
    if (!in_transaction_) {
        throw std::runtime_error("不在事务中");
    }
    const char* sql = "ROLLBACK TRANSACTION;";
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("回滚事务失败: " + err_str);
    }
    in_transaction_ = false;
}

int64_t DatabaseRepository::create_event(const model::Event& event) {
    std::ostringstream sql;
    sql << "INSERT INTO events (title, description, start_time, end_time, location, capacity, status, created_at, updated_at) VALUES (";
    sql << "'" << escape_string(event.title()) << "',";
    if (event.description()) {
        sql << "'" << escape_string(*event.description()) << "',";
    } else {
        sql << "NULL,";
    }
    sql << "'" << format_datetime(event.start_time()) << "',";
    sql << "'" << format_datetime(event.end_time()) << "',";
    sql << "'" << escape_string(event.location()) << "',";
    sql << event.capacity() << ",";
    sql << "'" << event.status_to_string() << "',";
    sql << "'" << format_datetime(event.created_at()) << "',";
    sql << "'" << format_datetime(event.updated_at()) << "');";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("创建活动失败: " + err_str);
    }

    return get_last_insert_id();
}

bool DatabaseRepository::update_event(const model::Event& event) {
    std::ostringstream sql;
    sql << "UPDATE events SET ";
    sql << "title = '" << escape_string(event.title()) << "',";
    if (event.description()) {
        sql << "description = '" << escape_string(*event.description()) << "',";
    } else {
        sql << "description = NULL,";
    }
    sql << "start_time = '" << format_datetime(event.start_time()) << "',";
    sql << "end_time = '" << format_datetime(event.end_time()) << "',";
    sql << "location = '" << escape_string(event.location()) << "',";
    sql << "capacity = " << event.capacity() << ",";
    sql << "status = '" << event.status_to_string() << "',";
    sql << "updated_at = '" << format_datetime(event.updated_at()) << "' ";
    sql << "WHERE id = " << event.id() << ";";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("更新活动失败: " + err_str);
    }

    return sqlite3_changes(db_) > 0;
}

std::optional<model::Event> DatabaseRepository::get_event_by_id(int64_t event_id) {
    std::ostringstream sql;
    sql << "SELECT id, title, description, start_time, end_time, location, capacity, status, created_at, updated_at FROM events WHERE id = " << event_id << ";";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询活动失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::optional<model::Event> result;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        model::Event event;
        event.set_id(sqlite3_column_int64(stmt, 0));
        event.set_title(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            event.set_description(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))));
        }
        event.set_start_time(parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))));
        event.set_end_time(parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))));
        event.set_location(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        event.set_capacity(sqlite3_column_int(stmt, 6));
        std::string status_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)));
        event.set_status(model::Event::string_to_status(status_str));
        event.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)));
        event.updated_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
        result = std::move(event);
    }

    sqlite3_finalize(stmt);
    return result;
}

EventListResult DatabaseRepository::get_events(const EventListFilter& filter, int page, int page_size) {
    std::ostringstream sql_count, sql_select;
    std::string conditions;

    // 构建WHERE条件
    if (filter.keyword) {
        if (!conditions.empty()) conditions += " AND ";
        conditions += "(title LIKE '%" + escape_string(*filter.keyword) + "%' OR description LIKE '%" + escape_string(*filter.keyword) + "%')";
    }

    if (filter.status) {
        if (!conditions.empty()) conditions += " AND ";
        model::Event event;
        event.set_status(*filter.status);
        conditions += "status = '" + event.status_to_string() + "'";
    }

    if (filter.from) {
        if (!conditions.empty()) conditions += " AND ";
        conditions += "start_time >= '" + format_datetime(*filter.from) + "'";
    }

    if (filter.to) {
        if (!conditions.empty()) conditions += " AND ";
        conditions += "start_time <= '" + format_datetime(*filter.to) + "'";
    }

    // 统计总数
    sql_count << "SELECT COUNT(*) FROM events";
    if (!conditions.empty()) {
        sql_count << " WHERE " << conditions;
    }
    sql_count << ";";

    sqlite3_stmt* stmt_count;
    int rc = sqlite3_prepare_v2(db_, sql_count.str().c_str(), -1, &stmt_count, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备统计活动总数失败: " + std::string(sqlite3_errmsg(db_)));
    }

    int total_count = 0;
    rc = sqlite3_step(stmt_count);
    if (rc == SQLITE_ROW) {
        total_count = sqlite3_column_int(stmt_count, 0);
    }
    sqlite3_finalize(stmt_count);

    // 查询列表
    sql_select << "SELECT id, title, description, start_time, end_time, location, capacity, status, created_at, updated_at FROM events";
    if (!conditions.empty()) {
        sql_select << " WHERE " << conditions;
    }
    sql_select << " ORDER BY created_at DESC";
    sql_select << " LIMIT " << page_size;
    sql_select << " OFFSET " << (page - 1) * page_size;
    sql_select << ";";

    sqlite3_stmt* stmt_select;
    rc = sqlite3_prepare_v2(db_, sql_select.str().c_str(), -1, &stmt_select, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询活动列表失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::vector<model::Event> events;
    while ((rc = sqlite3_step(stmt_select)) == SQLITE_ROW) {
        model::Event event;
        event.set_id(sqlite3_column_int64(stmt_select, 0));
        event.set_title(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 1)));
        if (sqlite3_column_type(stmt_select, 2) != SQLITE_NULL) {
            event.set_description(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 2))));
        }
        event.set_start_time(parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 3))));
        event.set_end_time(parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 4))));
        event.set_location(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 5)));
        event.set_capacity(sqlite3_column_int(stmt_select, 6));
        std::string status_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 7)));
        event.set_status(model::Event::string_to_status(status_str));
        event.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 8)));
        event.updated_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 9)));
        events.push_back(std::move(event));
    }

    sqlite3_finalize(stmt_select);

    return {events, total_count};
}

EventStats DatabaseRepository::get_event_stats(int64_t event_id) {
    std::ostringstream sql;
    sql << "SELECT COUNT(*) as total, ";
    sql << "SUM(CASE WHEN status = 'REGISTERED' THEN 1 ELSE 0 END) as registered, ";
    sql << "SUM(CASE WHEN status = 'WAITING' THEN 1 ELSE 0 END) as waiting, ";
    sql << "SUM(CASE WHEN status = 'CANCELED' THEN 1 ELSE 0 END) as canceled, ";
    sql << "SUM(CASE WHEN status = 'CHECKED_IN' THEN 1 ELSE 0 END) as checked_in ";
    sql << "FROM registrations WHERE event_id = " << event_id << ";";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询活动统计失败: " + std::string(sqlite3_errmsg(db_)));
    }

    EventStats stats{};
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        stats.total_registrations = sqlite3_column_int(stmt, 0);
        stats.registered_count = sqlite3_column_int(stmt, 1);
        stats.waiting_count = sqlite3_column_int(stmt, 2);
        stats.canceled_count = sqlite3_column_int(stmt, 3);
        stats.checked_in_count = sqlite3_column_int(stmt, 4);
    }

    sqlite3_finalize(stmt);
    return stats;
}

int64_t DatabaseRepository::create_user(const model::User& user) {
    std::ostringstream sql;
    sql << "INSERT INTO users (name, email, phone, created_at) VALUES (";
    sql << "'" << escape_string(user.name()) << "',";
    sql << "'" << escape_string(user.email()) << "',";
    if (user.phone()) {
        sql << "'" << escape_string(*user.phone()) << "',";
    } else {
        sql << "NULL,";
    }
    sql << "'" << format_datetime(user.created_at()) << "');";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        if (err_str.find("UNIQUE constraint failed: users.email") != std::string::npos) {
            throw std::runtime_error("邮箱已存在: " + user.email());
        }
        throw std::runtime_error("创建用户失败: " + err_str);
    }

    return get_last_insert_id();
}

std::optional<model::User> DatabaseRepository::get_user_by_id(int64_t user_id) {
    std::ostringstream sql;
    sql << "SELECT id, name, email, phone, created_at FROM users WHERE id = " << user_id << ";";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询用户失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::optional<model::User> result;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        model::User user;
        user.set_id(sqlite3_column_int64(stmt, 0));
        user.set_name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        user.set_email(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            user.set_phone(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))));
        }
        user.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        result = std::move(user);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::optional<model::User> DatabaseRepository::get_user_by_email(const std::string& email) {
    std::ostringstream sql;
    sql << "SELECT id, name, email, phone, created_at FROM users WHERE email = '" << escape_string(email) << "';";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询用户失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::optional<model::User> result;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        model::User user;
        user.set_id(sqlite3_column_int64(stmt, 0));
        user.set_name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        user.set_email(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            user.set_phone(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))));
        }
        user.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        result = std::move(user);
    }

    sqlite3_finalize(stmt);
    return result;
}

int64_t DatabaseRepository::create_registration(const model::Registration& registration) {
    std::ostringstream sql;
    sql << "INSERT INTO registrations (user_id, event_id, status, created_at, updated_at) VALUES (";
    sql << registration.user_id() << ",";
    sql << registration.event_id() << ",";
    sql << "'" << registration.status_to_string() << "',";
    sql << "'" << format_datetime(registration.created_at()) << "',";
    sql << "'" << format_datetime(registration.updated_at()) << "');";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("创建报名失败: " + err_str);
    }

    return get_last_insert_id();
}

bool DatabaseRepository::update_registration(const model::Registration& registration) {
    std::ostringstream sql;
    sql << "UPDATE registrations SET ";
    sql << "status = '" << registration.status_to_string() << "',";
    sql << "updated_at = '" << format_datetime(registration.updated_at()) << "' ";
    sql << "WHERE id = " << registration.id() << ";";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("更新报名失败: " + err_str);
    }

    return sqlite3_changes(db_) > 0;
}

std::optional<model::Registration> DatabaseRepository::get_registration_by_id(int64_t registration_id) {
    std::ostringstream sql;
    sql << "SELECT id, user_id, event_id, status, created_at, updated_at FROM registrations WHERE id = " << registration_id << ";";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询报名失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::optional<model::Registration> result;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        model::Registration registration;
        registration.set_id(sqlite3_column_int64(stmt, 0));
        registration.set_user_id(sqlite3_column_int64(stmt, 1));
        registration.set_event_id(sqlite3_column_int64(stmt, 2));
        std::string status_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        registration.set_status(model::Registration::string_to_status(status_str));
        registration.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        registration.updated_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        result = std::move(registration);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::optional<model::Registration> DatabaseRepository::get_registration_by_user_and_event(int64_t user_id, int64_t event_id) {
    std::ostringstream sql;
    sql << "SELECT id, user_id, event_id, status, created_at, updated_at FROM registrations WHERE user_id = " << user_id << " AND event_id = " << event_id << ";";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询报名失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::optional<model::Registration> result;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        model::Registration registration;
        registration.set_id(sqlite3_column_int64(stmt, 0));
        registration.set_user_id(sqlite3_column_int64(stmt, 1));
        registration.set_event_id(sqlite3_column_int64(stmt, 2));
        std::string status_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        registration.set_status(model::Registration::string_to_status(status_str));
        registration.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        registration.updated_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        result = std::move(registration);
    }

    sqlite3_finalize(stmt);
    return result;
}

RegistrationListResult DatabaseRepository::get_event_registrations(int64_t event_id, const RegistrationListFilter& filter, int page, int page_size) {
    std::ostringstream sql_count, sql_select;
    std::string conditions;

    // 构建WHERE条件
    conditions += "event_id = " + std::to_string(event_id);

    if (filter.status) {
        if (!conditions.empty()) conditions += " AND ";
        model::Registration registration;
        registration.set_status(*filter.status);
        conditions += "status = '" + registration.status_to_string() + "'";
    }

    // 统计总数
    sql_count << "SELECT COUNT(*) FROM registrations";
    if (!conditions.empty()) {
        sql_count << " WHERE " << conditions;
    }
    sql_count << ";";

    sqlite3_stmt* stmt_count;
    int rc = sqlite3_prepare_v2(db_, sql_count.str().c_str(), -1, &stmt_count, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备统计报名总数失败: " + std::string(sqlite3_errmsg(db_)));
    }

    int total_count = 0;
    rc = sqlite3_step(stmt_count);
    if (rc == SQLITE_ROW) {
        total_count = sqlite3_column_int(stmt_count, 0);
    }
    sqlite3_finalize(stmt_count);

    // 查询列表
    sql_select << "SELECT id, user_id, event_id, status, created_at, updated_at FROM registrations";
    if (!conditions.empty()) {
        sql_select << " WHERE " << conditions;
    }
    sql_select << " ORDER BY created_at ASC";
    sql_select << " LIMIT " << page_size;
    sql_select << " OFFSET " << (page - 1) * page_size;
    sql_select << ";";

    sqlite3_stmt* stmt_select;
    rc = sqlite3_prepare_v2(db_, sql_select.str().c_str(), -1, &stmt_select, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询报名列表失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::vector<model::Registration> registrations;
    while ((rc = sqlite3_step(stmt_select)) == SQLITE_ROW) {
        model::Registration registration;
        registration.set_id(sqlite3_column_int64(stmt_select, 0));
        registration.set_user_id(sqlite3_column_int64(stmt_select, 1));
        registration.set_event_id(sqlite3_column_int64(stmt_select, 2));
        std::string status_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 3)));
        registration.set_status(model::Registration::string_to_status(status_str));
        registration.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 4)));
        registration.updated_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt_select, 5)));
        registrations.push_back(std::move(registration));
    }

    sqlite3_finalize(stmt_select);

    return {registrations, total_count};
}

std::vector<model::Registration> DatabaseRepository::get_event_waiting_list(int64_t event_id) {
    std::ostringstream sql;
    sql << "SELECT id, user_id, event_id, status, created_at, updated_at FROM registrations WHERE event_id = " << event_id << " AND status = 'WAITING' ORDER BY created_at ASC;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询等候名单失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::vector<model::Registration> waiting_list;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        model::Registration registration;
        registration.set_id(sqlite3_column_int64(stmt, 0));
        registration.set_user_id(sqlite3_column_int64(stmt, 1));
        registration.set_event_id(sqlite3_column_int64(stmt, 2));
        registration.set_status(model::Registration::WAITING);
        registration.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        registration.updated_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        waiting_list.push_back(std::move(registration));
    }

    sqlite3_finalize(stmt);
    return waiting_list;
}

int DatabaseRepository::get_registered_count(int64_t event_id) {
    std::ostringstream sql;
    sql << "SELECT COUNT(*) FROM registrations WHERE event_id = " << event_id << " AND status = 'REGISTERED';";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询已报名人数失败: " + std::string(sqlite3_errmsg(db_)));
    }

    int count = 0;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

std::vector<model::Registration> DatabaseRepository::get_user_registrations(int64_t user_id) {
    std::ostringstream sql;
    sql << "SELECT id, user_id, event_id, status, created_at, updated_at FROM registrations WHERE user_id = " << user_id << " ORDER BY created_at DESC;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询用户报名记录失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::vector<model::Registration> registrations;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        model::Registration registration;
        registration.set_id(sqlite3_column_int64(stmt, 0));
        registration.set_user_id(sqlite3_column_int64(stmt, 1));
        registration.set_event_id(sqlite3_column_int64(stmt, 2));
        std::string status_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        registration.set_status(model::Registration::string_to_status(status_str));
        registration.created_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        registration.updated_at() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        registrations.push_back(std::move(registration));
    }

    sqlite3_finalize(stmt);
    return registrations;
}

int64_t DatabaseRepository::create_check_in_log(const model::CheckInLog& log) {
    std::ostringstream sql;
    sql << "INSERT INTO check_in_logs (registration_id, check_in_time, channel) VALUES (";
    sql << log.registration_id() << ",";
    sql << "'" << format_datetime(log.check_in_time()) << "',";
    sql << "'" << log.channel_to_string() << "');";

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK) {
        std::string err_str(err_msg);
        sqlite3_free(err_msg);
        throw std::runtime_error("创建签到日志失败: " + err_str);
    }

    return get_last_insert_id();
}

std::optional<model::CheckInLog> DatabaseRepository::get_check_in_log_by_registration(int64_t registration_id) {
    std::ostringstream sql;
    sql << "SELECT id, registration_id, check_in_time, channel FROM check_in_logs WHERE registration_id = " << registration_id << ";";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.str().c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("准备查询签到日志失败: " + std::string(sqlite3_errmsg(db_)));
    }

    std::optional<model::CheckInLog> result;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        model::CheckInLog log;
        log.set_id(sqlite3_column_int64(stmt, 0));
        log.set_registration_id(sqlite3_column_int64(stmt, 1));
        log.check_in_time() = parse_datetime(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        std::string channel_str(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        log.set_channel(model::CheckInLog::string_to_channel(channel_str));
        result = std::move(log);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseRepository::is_connected() const {
    return db_ != nullptr;
}

} // namespace event_signup_service::repository
