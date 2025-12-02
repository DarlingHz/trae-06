#ifndef BOOK_SERVICE_H
#define BOOK_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include "../model/Book.h"

class BookService {
public:
    /**
     * 新增图书
     * @param book 图书对象
     * @return 新增成功返回true，否则返回false
     */
    bool addBook(const Book& book);
    
    /**
     * 编辑图书信息
     * @param book 图书对象
     * @return 编辑成功返回true，否则返回false
     */
    bool editBook(const Book& book);
    
    /**
     * 下架图书
     * @param book_id 图书ID
     * @return 下架成功返回true，否则返回false
     */
    bool removeBook(int book_id);
    
    /**
     * 获取图书信息
     * @param book_id 图书ID
     * @return 获取成功返回图书对象，否则返回nullptr
     */
    std::shared_ptr<Book> getBookById(int book_id);
    
    /**
     * 搜索图书
     * @param keyword 搜索关键字
     * @param category 分类
     * @param page 页码
     * @param page_size 每页大小
     * @return 获取成功返回图书列表，否则返回空列表
     */
    std::vector<std::shared_ptr<Book>> searchBooks(const std::string& keyword, const std::string& category, int page, int page_size);
    
    /**
     * 获取所有图书
     * @return 获取成功返回图书列表，否则返回空列表
     */
    std::vector<std::shared_ptr<Book>> getAllBooks();
    
    /**
     * 获取图书总数
     * @param keyword 搜索关键字
     * @param category 分类
     * @return 获取成功返回图书总数，否则返回0
     */
    int getBookCount(const std::string& keyword, const std::string& category);
    
    /**
     * 更新图书库存
     * @param book_id 图书ID
     * @param total_quantity 总册数
     * @return 更新成功返回true，否则返回false
     */
    bool updateBookStock(int book_id, int total_quantity);
    
    /**
     * 增加图书在借数量
     * @param book_id 图书ID
     * @return 增加成功返回true，否则返回false
     */
    bool incrementBorrowCount(int book_id);
    
    /**
     * 减少图书在借数量
     * @param book_id 图书ID
     * @return 减少成功返回true，否则返回false
     */
    bool decrementBorrowCount(int book_id);
    
    /**
     * 检查图书是否可借
     * @param book_id 图书ID
     * @return 可借返回true，否则返回false
     */
    bool checkBookAvailable(int book_id);
};

#endif // BOOK_SERVICE_H