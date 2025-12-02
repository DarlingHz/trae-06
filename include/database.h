#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <queue>
#include <vector>
#include <map>
#include <mutex>
#include <condition_variable>
#include <memory>

namespace recruitment {

/**
 * @brief 数据库查询参数类型枚举
 */
enum class QueryParameterType {
    INTEGER,    ///< 整数类型
    TEXT,       ///< 文本类型
    REAL,       ///< 浮点数类型
    BLOB,       ///< 二进制数据类型
    NULL_TYPE   ///< NULL类型
};

/**
 * @brief 数据库查询参数结构体
 */
struct QueryParameter {
    QueryParameterType type;  ///< 参数类型
    long long int_value;      ///< 整数参数值
    double real_value;         ///< 浮点数参数值
    std::string text_value;    ///< 文本参数值
    std::vector<char> blob_value; ///< 二进制数据参数值

    /**
     * @brief 构造函数 - 整数类型参数
     */
    QueryParameter(long long value) : type(QueryParameterType::INTEGER), int_value(value) {}

    /**
     * @brief 构造函数 - 文本类型参数
     */
    QueryParameter(const std::string& value) : type(QueryParameterType::TEXT), text_value(value) {}

    /**
     * @brief 构造函数 - 浮点数类型参数
     */
    QueryParameter(double value) : type(QueryParameterType::REAL), real_value(value) {}

    /**
     * @brief 构造函数 - 二进制数据类型参数
     */
    QueryParameter(const std::vector<char>& value) : type(QueryParameterType::BLOB), blob_value(value) {}

    /**
     * @brief 构造函数 - NULL类型参数
     */
    QueryParameter() : type(QueryParameterType::NULL_TYPE) {}
};

/**
 * @brief 数据库查询结果行结构体
 */
struct QueryRow {
    /**
     * @brief 结果行数据存储
     * 键: 列名
     * 值: 列值（支持多种类型）
     */
    struct ColumnValue {
        long long int_value;      ///< 整数类型值
        double real_value;         ///< 浮点数类型值
        std::string text_value;    ///< 文本类型值
        std::vector<char> blob_value; ///< 二进制数据类型值
        bool is_null;              ///< 是否为NULL值

        /**
         * @brief 构造函数 - 初始化所有值为默认值
         */
        ColumnValue() : int_value(0), real_value(0.0), is_null(true) {}
    };

    std::map<std::string, ColumnValue> columns; ///< 列名到列值的映射

    /**
     * @brief 获取列值的引用
     * @param column_name 列名
     * @return 列值的引用
     * @throws std::out_of_range 如果列名不存在
     */
    const ColumnValue& operator[](const std::string& column_name) const {
        return columns.at(column_name);
    }

    /**
     * @brief 获取列值的引用（非const版本）
     * @param column_name 列名
     * @return 列值的引用
     * @throws std::out_of_range 如果列名不存在
     */
    ColumnValue& operator[](const std::string& column_name) {
        return columns[column_name];
    }
};

/**
 * @brief 数据库查询结果结构体
 */
struct QueryResult {
    std::vector<QueryRow> rows; ///< 查询结果行集合
    long long last_insert_id;    ///< 最后插入的ID（仅对INSERT语句有效）
    int rows_affected;            ///< 受影响的行数（仅对UPDATE/DELETE语句有效）

    /**
     * @brief 构造函数 - 初始化所有值为默认值
     */
    QueryResult() : last_insert_id(0), rows_affected(0) {}
};

/**
 * @brief 数据库连接类
 */
class DatabaseConnection {
public:
    /**
     * @brief 构造函数
     * @param db_path 数据库文件路径
     */
    explicit DatabaseConnection(const std::string& db_path);

    /**
     * @brief 析构函数
     */
    ~DatabaseConnection();

    /**
     * @brief 禁止拷贝构造
     */
    DatabaseConnection(const DatabaseConnection&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    /**
     * @brief 移动构造函数
     */
    DatabaseConnection(DatabaseConnection&& other) noexcept;

    /**
     * @brief 移动赋值操作符
     */
    DatabaseConnection& operator=(DatabaseConnection&& other) noexcept;

    /**
     * @brief 检查连接是否有效
     * @return 有效返回true，否则返回false
     */
    bool isValid() const;

    /**
     * @brief 执行SQL查询
     * @param sql SQL查询语句
     * @param callback 回调函数，处理查询结果
     * @param user_data 用户数据，传递给回调函数
     * @return 执行成功返回SQLITE_OK，否则返回错误码
     */
    int executeQuery(const std::string& sql, int (*callback)(void*, int, char**, char**), void* user_data);

    /**
     * @brief 执行SQL语句（INSERT、UPDATE、DELETE等）
     * @param sql SQL语句
     * @return 执行成功返回受影响的行数，否则返回-1
     */
    int executeNonQuery(const std::string& sql);

    /**
     * @brief 执行SQL语句并返回自增ID
     * @param sql SQL语句
     * @return 执行成功返回自增ID，否则返回-1
     */
    long long executeInsert(const std::string& sql);

    /**
     * @brief 执行带参数的SQL查询
     * @param sql SQL查询语句
     * @param parameters 查询参数
     * @return 查询结果
     */
    QueryResult executeQuery(const std::string& sql, const std::vector<QueryParameter>& parameters);

    /**
     * @brief 执行带参数的非查询SQL语句（INSERT、UPDATE、DELETE等）
     * @param sql SQL语句
     * @param parameters 查询参数
     * @return 受影响的行数
     */
    int executeNonQuery(const std::string& sql, const std::vector<QueryParameter>& parameters);

    /**
     * @brief 开始事务
     * @return 成功返回true，否则返回false
     */
    bool beginTransaction();

    /**
     * @brief 提交事务
     * @return 成功返回true，否则返回false
     */
    bool commitTransaction();

    /**
     * @brief 回滚事务
     * @return 成功返回true，否则返回false
     */
    bool rollbackTransaction();

    /**
     * @brief 获取SQLite3数据库连接指针
     * @return SQLite3数据库连接指针
     */
    sqlite3* getConnection() const;

private:
    /**
     * @brief 关闭数据库连接
     */
    void close();

    sqlite3* db_; ///< SQLite3数据库连接指针
    std::string db_path_; ///< 数据库文件路径
    bool is_valid_; ///< 连接是否有效
    bool is_in_transaction_; ///< 是否在事务中
};

/**
 * @brief 数据库连接池类
 */
class ConnectionPool {
public:
    /**
     * @brief 构造函数
     * @param db_path 数据库文件路径
     * @param max_connections 最大连接数
     */
    ConnectionPool(const std::string& db_path, int max_connections);

    /**
     * @brief 析构函数
     */
    ~ConnectionPool();

    /**
     * @brief 禁止拷贝构造
     */
    ConnectionPool(const ConnectionPool&) = delete;

    /**
     * @brief 禁止赋值操作
     */
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    /**
     * @brief 获取数据库连接
     * @param timeout_ms 超时时间（毫秒），默认30000毫秒
     * @return 数据库连接智能指针，如果超时返回空
     */
    std::shared_ptr<DatabaseConnection> getConnection(int timeout_ms = 30000);

    /**
     * @brief 放回数据库连接
     * @param connection 数据库连接智能指针
     */
    void returnConnection(std::shared_ptr<DatabaseConnection> connection);

    /**
     * @brief 获取连接池状态信息
     * @param total_connections 总连接数
     * @param available_connections 可用连接数
     * @param used_connections 使用中的连接数
     */
    void getStatus(int& total_connections, int& available_connections, int& used_connections) const;

private:
    /**
     * @brief 初始化连接池
     */
    void initialize();

    /**
     * @brief 关闭所有连接
     */
    void closeAllConnections();

    std::string db_path_; ///< 数据库文件路径
    int max_connections_; ///< 最大连接数
    int current_connections_; ///< 当前连接数
    std::queue<std::shared_ptr<DatabaseConnection>> available_connections_; ///< 可用连接队列
    std::vector<std::shared_ptr<DatabaseConnection>> all_connections_; ///< 所有连接集合
    mutable std::mutex mutex_; ///< 互斥锁，保护连接池状态
    std::condition_variable condition_; ///< 条件变量，用于等待连接可用
    bool is_initialized_; ///< 连接池是否已初始化
};

/**
 * @brief 数据库访问类
 */
class Database {
public:
    /**
     * @brief 初始化数据库
     * @param db_path 数据库文件路径
     * @param max_connections 最大连接数
     * @return 初始化成功返回true，否则返回false
     */
    static bool initialize(const std::string& db_path, int max_connections = 10);

    /**
     * @brief 获取数据库连接
     * @param timeout_ms 超时时间（毫秒）
     * @return 数据库连接智能指针，如果超时返回空
     */
    static std::shared_ptr<DatabaseConnection> getConnection(int timeout_ms = 30000);

    /**
     * @brief 放回数据库连接
     * @param connection 数据库连接智能指针
     */
    static void returnConnection(std::shared_ptr<DatabaseConnection> connection);

    /**
     * @brief 获取数据库连接池状态
     * @param total_connections 总连接数
     * @param available_connections 可用连接数
     * @param used_connections 使用中的连接数
     */
    static void getPoolStatus(int& total_connections, int& available_connections, int& used_connections);

    /**
     * @brief 执行数据库初始化脚本
     * @param sql_script SQL脚本内容
     * @return 执行成功返回true，否则返回false
     */
    static bool executeScript(const std::string& sql_script);

private:
    /**
     * @brief 构造函数
     */
    Database() = default;

    /**
     * @brief 析构函数
     */
    ~Database() = default;

    static std::unique_ptr<ConnectionPool> connection_pool_; ///< 数据库连接池
    static std::mutex mutex_; ///< 互斥锁，保护连接池初始化
};

} // namespace recruitment

#endif // DATABASE_H