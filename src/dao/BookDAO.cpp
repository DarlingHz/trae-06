#include "BookDAO.h"
#include "../util/Logger.h"
#include <stdexcept>

bool BookDAO::addBook(const Book& book) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for adding book");
            return false;
        }
        
        // 开始事务
        session->startTransaction();
        
        // 执行插入图书的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto insert_stmt = books_table.insert("title", "author", "isbn", "description", "total_quantity", "available_quantity", "borrowed_quantity", "status")
            .values(book.getTitle(), book.getAuthor(), book.getIsbn(), book.getDescription(), book.getTotalCopies(), book.getAvailableCopies(), book.getBorrowedCopies(), book.getStatus());
        
        mysqlx::Result result = insert_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to insert book into database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 获取插入的图书ID
        int book_id = result.getAutoIncrementValue();
        
        // 插入图书分类关系
        const std::vector<std::string>& categories = book.getCategories();
        if (!categories.empty()) {
            Table book_categories_table = session->getSchema("library_management_system").getTable("book_categories");
            
            for (const std::string& category : categories) {
                // 查询分类ID
                // 查询分类ID
                std::string category_sql = "SELECT id FROM categories WHERE name = ?";
                mysqlx::SqlStatement select_category_stmt = session->sql(category_sql)
                    .bind("category", category);
                
                mysqlx::SqlResult category_result = select_category_stmt.execute();
                std::vector<mysqlx::Row> category_rows = category_result.fetchAll();
                if (category_rows.empty()) {
                    Logger::error("Category not found: " + category);
                    session->rollback();
                    DatabaseConnectionPool::releaseConnection(session);
                    return false;
                }
                int category_id = category_rows[0][0].get<int>();
                
                // 插入图书分类关系
                auto insert_book_category_stmt = book_categories_table.insert("book_id", "category_id")
                    .values(book.getId(), category_id);
                
                insert_book_category_stmt.execute();
            }
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Book added successfully, book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to add book: " + std::string(e.what()));
        return false;
    }
}

bool BookDAO::updateBook(const Book& book) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for updating book");
            return false;
        }
        
        // 开始事务
        session->startTransaction();
        
        // 执行更新图书的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto update_stmt = books_table.update()
            .set("title", book.getTitle())
            .set("author", book.getAuthor())
            .set("isbn", book.getIsbn())
            .set("description", book.getDescription())
            .set("total_quantity", book.getTotalCopies())
            .set("available_quantity", book.getAvailableCopies())
            .set("borrowed_quantity", book.getBorrowedCopies())
            .set("status", book.getStatus())
            .where("id = :book_id")
            .bind("book_id", book.getId());
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to update book in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            session->rollback();
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        // 删除旧的图书分类关系
        std::string delete_sql = "DELETE FROM book_categories WHERE book_id = ?";
        mysqlx::SqlStatement delete_book_categories_stmt = session->sql(delete_sql)
            .bind("book_id", book.getId());
        
        delete_book_categories_stmt.execute();
        
        // 插入新的图书分类关系
        const std::vector<std::string>& categories = book.getCategories();
        if (!categories.empty()) {
            Table book_categories_table = session->getSchema("library_management_system").getTable("book_categories");
            for (const std::string& category : categories) {
                // 查询分类ID
                std::string category_sql = "SELECT id FROM categories WHERE name = ?";
                mysqlx::SqlStatement select_category_stmt = session->sql(category_sql)
                    .bind("category", category);
                
                mysqlx::SqlResult category_result = select_category_stmt.execute();
                std::vector<mysqlx::Row> category_rows = category_result.fetchAll();
                if (category_rows.empty()) {
                    Logger::error("Category not found: " + category);
                    session->rollback();
                    DatabaseConnectionPool::releaseConnection(session);
                    return false;
                }
                
                int category_id = category_rows[0][0].get<int>();
                
                // 插入图书分类关系
                auto insert_book_category_stmt = book_categories_table.insert("book_id", "category_id")
                    .values(book.getId(), category_id);
                
                insert_book_category_stmt.execute();
            }
        }
        
        // 提交事务
        session->commit();
        
        Logger::info("Book updated successfully, book id: " + std::to_string(book.getId()));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update book: " + std::string(e.what()));
        return false;
    }
}

bool BookDAO::removeBook(int book_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for removing book");
            return false;
        }
        
        // 执行更新图书状态的SQL语句（下架图书）
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto update_stmt = books_table.update()
            .set("status", "inactive")
            .where("id = :book_id")
            .bind("book_id", book_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to remove book from database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Book removed successfully, book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to remove book: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<Book> BookDAO::getBookById(int book_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting book by id");
            return nullptr;
        }
        
        // 执行查询图书的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto select_stmt = books_table.select("*")
            .where("id = :book_id")
            .bind("book_id", book_id);
        
        mysqlx::RowResult result = select_stmt.execute();
        std::vector<mysqlx::Row> result_rows = result.fetchAll();
        if (result_rows.empty()) {
            Logger::debug("Book not found by id: " + std::to_string(book_id));
            DatabaseConnectionPool::releaseConnection(session);
            return nullptr;
        }
        
        // 从结果集中创建Book对象
        mysqlx::Row row = result_rows[0];
        std::shared_ptr<Book> book = createBookFromResult(row);
        
        // 获取图书分类
        Table book_categories_table = session->getSchema("library_management_system").getTable("book_categories");
        Table categories_table = session->getSchema("library_management_system").getTable("categories");
        
        // 使用SQL语句执行join操作
        std::string sql = "SELECT categories.name FROM book_categories JOIN categories ON book_categories.category_id = categories.id WHERE book_categories.book_id = :book_id";
        mysqlx::SqlStatement select_categories_stmt = session->sql(sql)
            .bind("book_id", book_id);
        
        mysqlx::SqlResult categories_result = select_categories_stmt.execute();
        std::vector<std::string> categories;
        for (mysqlx::Row category_row : categories_result.fetchAll()) {
            categories.push_back(category_row[0].get<std::string>());
        }
        
        book->setCategories(categories);
        
        DatabaseConnectionPool::releaseConnection(session);
        return book;
    } catch (const std::exception& e) {
        Logger::error("Failed to get book by id: " + std::string(e.what()));
        return nullptr;
    }
}

std::shared_ptr<Book> BookDAO::getBookByISBN(const std::string& isbn) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting book by ISBN");
            return nullptr;
        }
        
        // 执行查询图书的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto select_stmt = books_table.select("*")
            .where("isbn = :isbn")
            .bind("isbn", isbn);
        
        mysqlx::RowResult result = select_stmt.execute();
        std::vector<mysqlx::Row> result_rows = result.fetchAll();
        if (result_rows.empty()) {
            Logger::debug("Book not found by ISBN: " + isbn);
            DatabaseConnectionPool::releaseConnection(session);
            return nullptr;
        }
        
        // 从结果集中创建Book对象
        mysqlx::Row row = result_rows[0];
        std::shared_ptr<Book> book = createBookFromResult(row);
        
        // 获取图书分类
        Table book_categories_table = session->getSchema("library_management_system").getTable("book_categories");
        Table categories_table = session->getSchema("library_management_system").getTable("categories");
        
        // 使用SQL语句执行join操作
        std::string sql = "SELECT categories.name FROM book_categories JOIN categories ON book_categories.category_id = categories.id WHERE book_categories.book_id = :book_id";
        mysqlx::SqlStatement select_categories_stmt = session->sql(sql)
            .bind("book_id", book->getId());
        
        mysqlx::SqlResult categories_result = select_categories_stmt.execute();
        std::vector<std::string> categories;
        for (mysqlx::Row category_row : categories_result.fetchAll()) {
            categories.push_back(category_row[0].get<std::string>());
        }
        
        book->setCategories(categories);
        
        DatabaseConnectionPool::releaseConnection(session);
        return book;
    } catch (const std::exception& e) {
        Logger::error("Failed to get book by ISBN: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::shared_ptr<Book>> BookDAO::searchBooks(const std::string& keyword, const std::string& category, int page, int page_size) {
    std::vector<std::shared_ptr<Book>> books;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for searching books");
            return books;
        }
        
        // 计算分页偏移量
        int offset = (page - 1) * page_size;
        
        // 构建查询语句
        std::string search_sql = "SELECT DISTINCT books.* FROM books";
        std::vector<std::string> params;
        std::vector<mysqlx::Value> values;
        
        // 添加关键字搜索条件
        if (!keyword.empty()) {
            search_sql += " WHERE (books.title LIKE ? OR books.author LIKE ? OR books.isbn LIKE ?)";
            params.push_back("keyword");
            values.push_back("%" + keyword + "%");
        }
        
        // 添加分类过滤条件
        if (!category.empty()) {
            Table book_categories_table = session->getSchema("library_management_system").getTable("book_categories");
            Table categories_table = session->getSchema("library_management_system").getTable("categories");
            
            // 使用SQL语句执行join操作
            std::string sql = "SELECT DISTINCT books.* FROM books JOIN book_categories ON books.id = book_categories.book_id JOIN categories ON book_categories.category_id = categories.id WHERE categories.name = :category";
            if (!keyword.empty()) {
                sql += " AND (books.title LIKE :keyword OR books.author LIKE :keyword OR books.isbn LIKE :keyword)";
            }
            sql += " ORDER BY books.id LIMIT :page_size OFFSET :offset";
            
            mysqlx::SqlStatement sql_stmt = session->sql(sql)
                .bind("category", category)
                .bind("page_size", page_size)
                .bind("offset", offset);
            
            if (!keyword.empty()) {
                sql_stmt.bind("keyword", "%" + keyword + "%");
            }
            
            // 执行查询语句
            mysqlx::SqlResult result = sql_stmt.execute();
            std::vector<mysqlx::Row> result_rows = result.fetchAll();
            
            // 从结果集中创建Book对象
            for (const mysqlx::Row& row : result_rows) {
                std::shared_ptr<Book> book = createBookFromResult(row);
                
                // 获取图书分类
                std::string category_sql = "SELECT categories.name FROM book_categories JOIN categories ON book_categories.category_id = categories.id WHERE book_categories.book_id = :book_id";
                mysqlx::SqlStatement category_stmt = session->sql(category_sql)
                    .bind("book_id", book->getId());
                
                mysqlx::SqlResult category_result = category_stmt.execute();
                std::vector<std::string> categories;
                for (mysqlx::Row category_row : category_result.fetchAll()) {
                    categories.push_back(category_row[0].get<std::string>());
                }
                
                book->setCategories(categories);
                books.push_back(book);
            }
            
            DatabaseConnectionPool::releaseConnection(session);
            return books;
        }
        
        // 添加分类过滤条件
        if (!category.empty()) {
            search_sql += " JOIN book_categories ON books.id = book_categories.book_id JOIN categories ON book_categories.category_id = categories.id WHERE categories.name = ?";
            params.push_back("category");
            values.push_back(category);
        }
        
        // 添加分页条件
        search_sql += " ORDER BY books.id LIMIT ? OFFSET ?";
        params.push_back("page_size");
        values.push_back(page_size);
        params.push_back("offset");
        values.push_back(offset);
        
        // 执行查询语句
        mysqlx::SqlStatement sql_stmt = session->sql(search_sql);
        for (size_t i = 0; i < params.size(); i++) {
            sql_stmt.bind(params[i], values[i]);
        }
        
        mysqlx::SqlResult result = sql_stmt.execute();
        std::vector<mysqlx::Row> result_rows = result.fetchAll();
        
        // 从结果集中创建Book对象
        for (const mysqlx::Row& row : result_rows) {
            std::shared_ptr<Book> book = createBookFromResult(row);
            
            // 获取图书分类
            std::string category_sql = "SELECT categories.name FROM book_categories JOIN categories ON book_categories.category_id = categories.id WHERE book_categories.book_id = ?";
            mysqlx::SqlStatement category_stmt = session->sql(category_sql)
                .bind("book_id", book->getId());
            
            mysqlx::SqlResult category_result = category_stmt.execute();
            std::vector<std::string> categories;
            for (mysqlx::Row category_row : category_result.fetchAll()) {
                categories.push_back(category_row[0].get<std::string>());
            }
            
            book->setCategories(categories);
            books.push_back(book);
        }

        
        DatabaseConnectionPool::releaseConnection(session);
        return books;
    } catch (const std::exception& e) {
        Logger::error("Failed to search books: " + std::string(e.what()));
        return books;
    }
}

std::vector<std::shared_ptr<Book>> BookDAO::getAllBooks(int page, int page_size) {
    std::vector<std::shared_ptr<Book>> books;
    
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting all books");
            return books;
        }
        
        // 计算分页偏移量
        int offset = (page - 1) * page_size;
        
        // 执行查询所有图书的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto select_stmt = books_table.select("*")
            .orderBy("id")
            .limit(page_size)
            .offset(offset);
        
        mysqlx::RowResult result = select_stmt.execute();
        std::vector<mysqlx::Row> result_rows = result.fetchAll();
        if (result_rows.empty()) {
            Logger::debug("No books found");
            DatabaseConnectionPool::releaseConnection(session);
            return books;
        }
        
        // 从结果集中创建Book对象
        for (const mysqlx::Row& row : result_rows) {
            std::shared_ptr<Book> book = createBookFromResult(row);
            
            // 获取图书分类
            Table book_categories_table = session->getSchema("library_management_system").getTable("book_categories");
            Table categories_table = session->getSchema("library_management_system").getTable("categories");
            
            // 使用SQL语句执行join操作
            std::string category_sql = "SELECT categories.name FROM book_categories JOIN categories ON book_categories.category_id = categories.id WHERE book_categories.book_id = :book_id";
            mysqlx::SqlStatement select_categories_stmt = session->sql(category_sql)
                .bind("book_id", book->getId());
            
            mysqlx::SqlResult categories_result = select_categories_stmt.execute();
            std::vector<std::string> categories;
            for (mysqlx::Row category_row : categories_result.fetchAll()) {
                categories.push_back(category_row[0].get<std::string>());
            }
            
            book->setCategories(categories);
            books.push_back(book);
        }
        
        DatabaseConnectionPool::releaseConnection(session);
        return books;
    } catch (const std::exception& e) {
        Logger::error("Failed to get all books: " + std::string(e.what()));
        return books;
    }
}

int BookDAO::getBookCount(const std::string& keyword, const std::string& category) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for getting book count");
            return 0;
        }
        
        // 构建查询语句
        std::string count_sql = "SELECT COUNT(DISTINCT books.id) FROM books";
        std::vector<std::string> params;
        std::vector<mysqlx::Value> values;
        
        // 添加关键字搜索条件
        if (!keyword.empty()) {
            count_sql += " WHERE (books.title LIKE ? OR books.author LIKE ? OR books.isbn LIKE ?)";
            params.push_back("keyword");
            values.push_back("%" + keyword + "%");
        }
        
        // 添加分类过滤条件
        if (!category.empty()) {
            if (keyword.empty()) {
                count_sql += " WHERE";
            } else {
                count_sql += " AND";
            }
            count_sql += " books.id IN (SELECT book_categories.book_id FROM book_categories JOIN categories ON book_categories.category_id = categories.id WHERE categories.name = ?)";
            params.push_back("category");
            values.push_back(category);
        }
        
        // 执行查询语句
        mysqlx::SqlStatement select_stmt = session->sql(count_sql);
        for (size_t i = 0; i < params.size(); i++) {
            select_stmt.bind(params[i], values[i]);
        }
        mysqlx::SqlResult result = select_stmt.execute();
        std::vector<mysqlx::Row> result_rows = result.fetchAll();
        if (result_rows.empty()) {
            Logger::debug("No books found for count criteria");
            DatabaseConnectionPool::releaseConnection(session);
            return 0;
        }
        
        // 获取图书总数
        mysqlx::Row row = result_rows[0];
        int book_count = row[0].get<int>();
        
        DatabaseConnectionPool::releaseConnection(session);
        return book_count;
    } catch (const std::exception& e) {
        Logger::error("Failed to get book count: " + std::string(e.what()));
        return 0;
    }
}

bool BookDAO::updateBookStock(int book_id, int total_quantity, int available_quantity, int borrowed_quantity) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for updating book stock");
            return false;
        }
        
        // 执行更新图书库存的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto update_stmt = books_table.update()
            .set("total_quantity", total_quantity)
            .set("available_quantity", available_quantity)
            .set("borrowed_quantity", borrowed_quantity)
            .where("id = :book_id")
            .bind("book_id", book_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to update book stock in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Book stock updated successfully, book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to update book stock: " + std::string(e.what()));
        return false;
    }
}

bool BookDAO::decreaseAvailableQuantity(int book_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for decreasing available quantity");
            return false;
        }
        
        // 执行减少图书可借数量的SQL语句（使用行级锁保证并发安全）
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto update_stmt = books_table.update()
            .set("available_quantity", "available_quantity - 1")
            .where("id = :book_id AND available_quantity > 0")
            .bind("book_id", book_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to decrease available quantity in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Available quantity decreased successfully, book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to decrease available quantity: " + std::string(e.what()));
        return false;
    }
}

bool BookDAO::increaseAvailableQuantity(int book_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for increasing available quantity");
            return false;
        }
        
        // 执行增加图书可借数量的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto update_stmt = books_table.update()
            .set("available_quantity", "available_quantity + 1")
            .where("id = :book_id")
            .bind("book_id", book_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to increase available quantity in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Available quantity increased successfully, book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to increase available quantity: " + std::string(e.what()));
        return false;
    }
}

bool BookDAO::increaseBorrowedQuantity(int book_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for increasing borrowed quantity");
            return false;
        }
        
        // 执行增加图书在借数量的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto update_stmt = books_table.update()
            .set("borrowed_quantity", "borrowed_quantity + 1")
            .where("id = :book_id")
            .bind("book_id", book_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to increase borrowed quantity in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Borrowed quantity increased successfully, book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to increase borrowed quantity: " + std::string(e.what()));
        return false;
    }
}

bool BookDAO::decreaseBorrowedQuantity(int book_id) {
    try {
        // 获取数据库连接
        std::shared_ptr<Session> session = DatabaseConnectionPool::getConnection();
        if (!session) {
            Logger::error("Failed to get database connection for decreasing borrowed quantity");
            return false;
        }
        
        // 执行减少图书在借数量的SQL语句
        Table books_table = session->getSchema("library_management_system").getTable("books");
        auto update_stmt = books_table.update()
            .set("borrowed_quantity", "borrowed_quantity - 1")
            .where("id = :book_id AND borrowed_quantity > 0")
            .bind("book_id", book_id);
        
        mysqlx::Result result = update_stmt.execute();
        if (result.getAffectedItemsCount() != 1) {
            Logger::error("Failed to decrease borrowed quantity in database, affected rows: " + std::to_string(result.getAffectedItemsCount()));
            DatabaseConnectionPool::releaseConnection(session);
            return false;
        }
        
        Logger::info("Borrowed quantity decreased successfully, book id: " + std::to_string(book_id));
        DatabaseConnectionPool::releaseConnection(session);
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to decrease borrowed quantity: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<Book> BookDAO::createBookFromResult(const mysqlx::Row& row) {
    int id = row[0].get<int>();
    std::string title = row[1].get<std::string>();
    std::string author = row[2].get<std::string>();
    std::string isbn = row[3].get<std::string>();
    std::string description = row[4].get<std::string>();
    int total_quantity = row[5].get<int>();
    int available_quantity = row[6].get<int>();
    int borrowed_quantity = row[7].get<int>();
    std::string status = row[8].get<std::string>();
    std::string created_at = row[9].get<std::string>();
    std::string updated_at = row[10].get<std::string>();
    
    return std::make_shared<Book>(id, title, author, isbn, description, total_quantity, available_quantity, borrowed_quantity, status, created_at, updated_at);
}