#ifndef BOOKDAO_H
#define BOOKDAO_H

#include "../model/Book.h"
#include "../util/DatabaseConnectionPool.h"
#include <vector>
#include <memory>

class BookDAO {
public:
    // 新增图书
    static bool addBook(const Book& book);
    
    // 编辑图书信息
    static bool updateBook(const Book& book);
    
    // 下架图书
    static bool removeBook(int book_id);
    
    // 根据ID获取图书信息
    static std::shared_ptr<Book> getBookById(int book_id);
    
    // 根据ISBN获取图书信息
    static std::shared_ptr<Book> getBookByISBN(const std::string& isbn);
    
    // 搜索图书
    static std::vector<std::shared_ptr<Book>> searchBooks(const std::string& keyword, const std::string& category = "", int page = 1, int page_size = 10);
    
    // 获取所有图书
    static std::vector<std::shared_ptr<Book>> getAllBooks(int page = 1, int page_size = 10);
    
    // 获取图书总数
    static int getBookCount(const std::string& keyword = "", const std::string& category = "");
    
    // 更新图书库存
    static bool updateBookStock(int book_id, int total_quantity, int available_quantity, int borrowed_quantity);
    
    // 减少图书可借数量
    static bool decreaseAvailableQuantity(int book_id);
    
    // 增加图书可借数量
    static bool increaseAvailableQuantity(int book_id);
    
    // 增加图书在借数量
    static bool increaseBorrowedQuantity(int book_id);
    
    // 减少图书在借数量
    static bool decreaseBorrowedQuantity(int book_id);
    
private:
    // 从数据库结果集中创建Book对象
    static std::shared_ptr<Book> createBookFromResult(const Row& row);
};

#endif // BOOKDAO_H