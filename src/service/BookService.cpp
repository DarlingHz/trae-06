#include "BookService.h"
#include "../dao/BookDAO.h"
#include "../util/Logger.h"
#include <stdexcept>

bool BookService::addBook(const Book& book) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 新增图书
        bool result = book_dao.addBook(book);
        
        if (result) {
            Logger::info("Book added successfully: " + book.getTitle());
        } else {
            Logger::error("Failed to add book: " + book.getTitle());
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to add book: " + std::string(e.what()));
        return false;
    }
}

bool BookService::editBook(const Book& book) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 编辑图书信息
        bool result = book_dao.updateBook(book);
        
        if (result) {
            Logger::info("Book edited successfully: " + book.getTitle());
        } else {
            Logger::error("Failed to edit book: " + book.getTitle());
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to edit book: " + std::string(e.what()));
        return false;
    }
}

bool BookService::removeBook(int book_id) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 下架图书
        bool result = book_dao.removeBook(book_id);
        
        if (result) {
            Logger::info("Book removed successfully: " + std::to_string(book_id));
        } else {
            Logger::error("Failed to remove book: " + std::to_string(book_id));
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to remove book: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<Book> BookService::getBookById(int book_id) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 获取图书信息
        std::shared_ptr<Book> book = book_dao.getBookById(book_id);
        
        if (book) {
            Logger::info("Book retrieved successfully: " + std::to_string(book_id));
        } else {
            Logger::error("Failed to retrieve book: " + std::to_string(book_id));
        }
        
        return book;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve book: " + std::string(e.what()));
        return nullptr;
    }
}

std::vector<std::shared_ptr<Book>> BookService::searchBooks(const std::string& keyword, const std::string& category, int page, int page_size) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 搜索图书
        std::vector<std::shared_ptr<Book>> books = book_dao.searchBooks(keyword, category, page, page_size);
        
        Logger::info("Books searched successfully, keyword: " + keyword + ", category: " + category + ", page: " + std::to_string(page) + ", page size: " + std::to_string(page_size));
        return books;
    } catch (const std::exception& e) {
        Logger::error("Failed to search books: " + std::string(e.what()));
        return std::vector<std::shared_ptr<Book>>();
    }
}

std::vector<std::shared_ptr<Book>> BookService::getAllBooks() {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 获取所有图书
        std::vector<std::shared_ptr<Book>> books = book_dao.getAllBooks();
        
        Logger::info("All books retrieved successfully");
        return books;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve all books: " + std::string(e.what()));
        return std::vector<std::shared_ptr<Book>>();
    }
}

int BookService::getBookCount(const std::string& keyword, const std::string& category) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 获取图书总数
        int count = book_dao.getBookCount(keyword, category);
        
        Logger::info("Book count retrieved successfully, keyword: " + keyword + ", category: " + category);
        return count;
    } catch (const std::exception& e) {
        Logger::error("Failed to retrieve book count: " + std::string(e.what()));
        return 0;
    }
}

bool BookService::updateBookStock(int book_id, int total_quantity) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 获取图书信息
        std::shared_ptr<Book> book = book_dao.getBookById(book_id);
        if (!book) {
            Logger::error("Book not found: " + std::to_string(book_id));
            return false;
        }
        
        // 检查库存数据一致性
        int current_borrowed = book->getBorrowedCopies();
        
        if (total_quantity < current_borrowed) {
            Logger::error("Total quantity cannot be less than borrowed quantity: " + std::to_string(book_id));
            return false;
        }
        
        // 计算新的可借数量
        int new_available = total_quantity - current_borrowed;
        
        // 更新图书库存
        bool result = book_dao.updateBookStock(book_id, total_quantity, new_available, current_borrowed);
        
        if (result) {
            Logger::info("Book stock updated successfully: " + std::to_string(book_id) + ", total quantity: " + std::to_string(total_quantity));
        } else {
            Logger::error("Failed to update book stock: " + std::to_string(book_id));
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to update book stock: " + std::string(e.what()));
        return false;
    }
}

bool BookService::incrementBorrowCount(int book_id) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 获取图书信息
        std::shared_ptr<Book> book = book_dao.getBookById(book_id);
        if (!book) {
            Logger::error("Book not found: " + std::to_string(book_id));
            return false;
        }
        
        // 计算新的库存数量
        int new_total = book->getTotalCopies();
        int new_available = book->getAvailableCopies() - 1;
        int new_borrowed = book->getBorrowedCopies() + 1;
        
        // 更新图书库存
        bool result = book_dao.updateBookStock(book_id, new_total, new_available, new_borrowed);
        
        if (result) {
            Logger::info("Book borrow count incremented successfully: " + std::to_string(book_id));
        } else {
            Logger::error("Failed to increment book borrow count: " + std::to_string(book_id));
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to increment book borrow count: " + std::string(e.what()));
        return false;
    }
}

bool BookService::decrementBorrowCount(int book_id) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 获取图书信息
        std::shared_ptr<Book> book = book_dao.getBookById(book_id);
        if (!book) {
            Logger::error("Book not found: " + std::to_string(book_id));
            return false;
        }
        
        // 计算新的库存数量
        int new_total = book->getTotalCopies();
        int new_available = book->getAvailableCopies() + 1;
        int new_borrowed = book->getBorrowedCopies() - 1;
        
        // 更新图书库存
        bool result = book_dao.updateBookStock(book_id, new_total, new_available, new_borrowed);
        
        if (result) {
            Logger::info("Book borrow count decremented successfully: " + std::to_string(book_id));
        } else {
            Logger::error("Failed to decrement book borrow count: " + std::to_string(book_id));
        }
        
        return result;
    } catch (const std::exception& e) {
        Logger::error("Failed to decrement book borrow count: " + std::string(e.what()));
        return false;
    }
}

bool BookService::checkBookAvailable(int book_id) {
    try {
        // 创建BookDAO对象
        BookDAO book_dao;
        
        // 获取图书信息
        std::shared_ptr<Book> book = book_dao.getBookById(book_id);
        if (!book) {
            Logger::error("Book not found: " + std::to_string(book_id));
            return false;
        }
        
        // 检查图书是否可借
        if (book->getStatus() != "active" || book->getAvailableCopies() <= 0) {
            Logger::error("Book not available: " + std::to_string(book_id));
            return false;
        }
        
        Logger::info("Book available check passed: " + std::to_string(book_id));
        return true;
    } catch (const std::exception& e) {
        Logger::error("Failed to check book available: " + std::string(e.what()));
        return false;
    }
}