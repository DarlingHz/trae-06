#include "BookController.h"
#include <cpprest/json.h>
#include <string>
#include <vector>
#include <memory>
#include "../service/BookService.h"
#include "../service/UserService.h"
#include "../model/Book.h"
#include "../model/User.h"

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;
using namespace std;


BookController::BookController(const utility::string_t& url)
    : listener(url), book_service(make_shared<BookService>()), user_service(make_shared<UserService>()), borrow_service(make_shared<BorrowService>()), reservation_service(make_shared<ReservationService>()) {
    listener.support(methods::GET, bind(&BookController::handleGet, this, placeholders::_1));
    listener.support(methods::POST, bind(&BookController::handlePost, this, placeholders::_1));
    listener.support(methods::PUT, bind(&BookController::handlePut, this, placeholders::_1));
    listener.support(methods::DEL, bind(&BookController::handleDelete, this, placeholders::_1));
}

BookController::~BookController() {
    stop();
}

void BookController::start() {
    listener.open().wait();
}

void BookController::stop() {
    listener.close().wait();
}

void BookController::handleGet(http_request request) {
    utility::string_t path = request.relative_uri().path();
    vector<utility::string_t> segments = uri::split_path(path);

    if (segments.size() == 2 && segments[1] == U("search")) {
        handleSearchBooks(request);
    } else if (segments.size() == 2) {
        handleGetBookById(request, segments[1]);
    } else {
        handleGetAllBooks(request);
    }
}

void BookController::handlePost(http_request request) {
    utility::string_t path = request.relative_uri().path();
    vector<utility::string_t> segments = uri::split_path(path);

    if (segments.size() == 2 && segments[1] == U("borrow")) {
        handleBorrowBook(request);
    } else if (segments.size() == 2 && segments[1] == U("return")) {
        handleReturnBook(request);
    } else if (segments.size() == 2 && segments[1] == U("reserve")) {
        handleReserveBook(request);
    } else {
        handleAddBook(request);
    }
}

void BookController::handlePut(http_request request) {
    utility::string_t path = request.relative_uri().path();
    vector<utility::string_t> segments = uri::split_path(path);

    if (segments.size() == 2) {
        handleUpdateBook(request, segments[1]);
    } else {
        sendResponse(request, status_codes::BadRequest, 400, "无效的请求路径");
    }
}

void BookController::handleDelete(http_request request) {
    utility::string_t path = request.relative_uri().path();
    vector<utility::string_t> segments = uri::split_path(path);

    if (segments.size() == 2) {
        handleDeleteBook(request, segments[1]);
    } else {
        sendResponse(request, status_codes::BadRequest, 400, "无效的请求路径");
    }
}

void BookController::handleGetAllBooks(http_request request) {
    try {
        vector<shared_ptr<Book>> books = book_service->searchBooks("", "", 1, INT_MAX);
        web::json::value response = web::json::value::array();

        for (size_t i = 0; i < books.size(); ++i) {
            response[i] = serializeBook(books[i]);
        }

        sendResponse(request, status_codes::OK, 200, "获取书籍列表成功", response);
    } catch (const exception& e) {
        sendResponse(request, status_codes::InternalError, 500, "获取书籍列表失败: " + string(e.what()));
    }
}

void BookController::handleGetBookById(http_request request, const utility::string_t& book_id) {
    try {
        int id = stoi(utility::conversions::to_utf8string(book_id));
        shared_ptr<Book> book = book_service->getBookById(id);

        if (book) {
            sendResponse(request, status_codes::OK, 200, "获取书籍信息成功", serializeBook(book));
        } else {
            sendResponse(request, status_codes::NotFound, 404, "书籍不存在");
        }
    } catch (const invalid_argument& e) {
        sendResponse(request, status_codes::BadRequest, 400, "无效的书籍ID");
    } catch (const exception& e) {
        sendResponse(request, status_codes::InternalError, 500, "获取书籍信息失败: " + string(e.what()));
    }
}

void BookController::handleAddBook(http_request request) {
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 验证请求参数
            if (!body.has_field("title") || !body.has_field("author") || !body.has_field("isbn") || !body.has_field("category_id") || !body.has_field("total_count")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }

            string title = body["title"].as_string();
            string author = body["author"].as_string();
            // Book类中没有publisher和publish_date属性，移除相关代码
            string isbn = body["isbn"].as_string();
            int category_id = body["category_id"].as_integer();
            int total_count = body["total_count"].as_integer();

            // 创建书籍对象
            shared_ptr<Book> book = make_shared<Book>();
            book->setTitle(title);
            book->setAuthor(author);
            // Book类中没有publisher和publish_date属性，移除相关代码
            book->setIsbn(isbn);
            vector<string> categories;
            categories.push_back(to_string(category_id));
            book->setCategories(categories);
            book->setTotalCopies(total_count);
            book->setAvailableCopies(total_count);

            // 添加书籍
            bool success = book_service->addBook(*book);

            if (success) {
                sendResponse(request, status_codes::Created, 201, "添加书籍成功");
            } else {
                sendResponse(request, status_codes::InternalError, 500, "添加书籍失败");
            }
        } catch (const exception& e) {
            sendResponse(request, status_codes::InternalError, 500, "添加书籍失败: " + string(e.what()));
        }
    }).wait();
}

void BookController::handleUpdateBook(http_request request, const utility::string_t& book_id) {
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            int id = stoi(utility::conversions::to_utf8string(book_id));

            // 验证请求参数
            if (!body.has_field("title") || !body.has_field("author") || !body.has_field("isbn") || !body.has_field("category_id") || !body.has_field("total_count")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }

            string title = body["title"].as_string();
            string author = body["author"].as_string();
            // Book类中没有publisher和publish_date属性，移除相关代码
            string isbn = body["isbn"].as_string();
            int category_id = body["category_id"].as_integer();
            int total_count = body["total_count"].as_integer();

            // 创建书籍对象
            shared_ptr<Book> book = make_shared<Book>();
            book->setId(id);
            book->setTitle(title);
            book->setAuthor(author);
            // Book类中没有publisher和publish_date属性，移除相关代码
            book->setIsbn(isbn);
            vector<string> categories;
            categories.push_back(to_string(category_id));
            book->setCategories(categories);
            book->setTotalCopies(total_count);

            // 更新书籍
            bool success = book_service->editBook(*book);

            if (success) {
                sendResponse(request, status_codes::OK, 200, "更新书籍成功");
            } else {
                sendResponse(request, status_codes::InternalError, 500, "更新书籍失败");
            }
        } catch (const invalid_argument& e) {
            sendResponse(request, status_codes::BadRequest, 400, "无效的书籍ID");
        } catch (const exception& e) {
            sendResponse(request, status_codes::InternalError, 500, "更新书籍失败: " + string(e.what()));
        }
    }).wait();
}

void BookController::handleDeleteBook(http_request request, const utility::string_t& book_id) {
    try {
        int id = stoi(utility::conversions::to_utf8string(book_id));

        // 删除书籍
        bool success = book_service->removeBook(id);

        if (success) {
            sendResponse(request, status_codes::OK, 200, "删除书籍成功");
        } else {
            sendResponse(request, status_codes::InternalError, 500, "删除书籍失败");
        }
    } catch (const invalid_argument& e) {
        sendResponse(request, status_codes::BadRequest, 400, "无效的书籍ID");
    } catch (const exception& e) {
        sendResponse(request, status_codes::InternalError, 500, "删除书籍失败: " + string(e.what()));
    }
}

void BookController::handleSearchBooks(http_request request) {
    try {
        // 获取查询参数
        uri_builder builder(request.relative_uri());
        auto query = builder.query();
        auto params = uri::split_query(query);

        // 解析查询参数
        string keyword;
        string category;
        string sort_by = "id";
        string sort_order = "asc";
        int page = 1;
        int page_size = 10;

        if (params.find(U("keyword")) != params.end()) {
            keyword = utility::conversions::to_utf8string(params[U("keyword")]);
        }

        if (params.find(U("category")) != params.end()) {
            category = utility::conversions::to_utf8string(params[U("category")]);
        }

        if (params.find(U("sort_by")) != params.end()) {
            sort_by = utility::conversions::to_utf8string(params[U("sort_by")]);
        }

        if (params.find(U("sort_order")) != params.end()) {
            sort_order = utility::conversions::to_utf8string(params[U("sort_order")]);
        }

        if (params.find(U("page")) != params.end()) {
            page = stoi(utility::conversions::to_utf8string(params[U("page")]));
        }

        if (params.find(U("page_size")) != params.end()) {
            page_size = stoi(utility::conversions::to_utf8string(params[U("page_size")]));
        }

        // 搜索书籍
        vector<shared_ptr<Book>> books = book_service->searchBooks(keyword, category, page, page_size);
        int total = book_service->getBookCount(keyword, category);

        // 构建响应
        web::json::value response = web::json::value::object();
        response["code"] = web::json::value::number(200);
        response["message"] = web::json::value::string("搜索书籍成功");
        response["data"] = web::json::value::object();
        response["data"]["books"] = web::json::value::array();

        for (size_t i = 0; i < books.size(); ++i) {
            response["data"]["books"][i] = serializeBook(books[i]);
        }

        response["data"]["pagination"] = web::json::value::object();
        response["data"]["pagination"]["page"] = web::json::value::number(page);
        response["data"]["pagination"]["page_size"] = web::json::value::number(page_size);
        response["data"]["pagination"]["total"] = web::json::value::number(total);
        response["data"]["pagination"]["pages"] = web::json::value::number((total + page_size - 1) / page_size);

        request.reply(status_codes::OK, response);
    } catch (const exception& e) {
        sendResponse(request, status_codes::InternalError, 500, "搜索书籍失败: " + string(e.what()));
    }
}

void BookController::handleBorrowBook(http_request request) {
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 验证请求参数
            if (!body.has_field("user_id") || !body.has_field("book_id")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }

            int user_id = body["user_id"].as_integer();
            int book_id = body["book_id"].as_integer();

            // 验证用户是否存在且状态正常
            shared_ptr<User> user = user_service->getUserInfo(user_id);
            if (!user) {
                sendResponse(request, status_codes::BadRequest, 400, "用户不存在");
                return;
            }

            if (user->getStatus() != "active") {
                sendResponse(request, status_codes::BadRequest, 400, "用户状态异常，无法借阅书籍");
                return;
            }

            // 借阅书籍
            int borrow_id = borrow_service->borrowBook(user_id, book_id);
            bool success = (borrow_id != -1);

            if (success) {
                sendResponse(request, status_codes::OK, 200, "借阅书籍成功");
            } else {
                sendResponse(request, status_codes::InternalError, 500, "借阅书籍失败");
            }
        } catch (const exception& e) {
            sendResponse(request, status_codes::InternalError, 500, "借阅书籍失败: " + string(e.what()));
        }
    }).wait();
}

void BookController::handleReturnBook(http_request request) {
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 验证请求参数
            if (!body.has_field("user_id") || !body.has_field("book_id")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }

            int user_id = body["user_id"].as_integer();
            int book_id = body["book_id"].as_integer();

            // 验证用户是否存在且状态正常
            shared_ptr<User> user = user_service->getUserInfo(user_id);
            if (!user) {
                sendResponse(request, status_codes::BadRequest, 400, "用户不存在");
                return;
            }

            if (user->getStatus() != "active") {
                sendResponse(request, status_codes::BadRequest, 400, "用户状态异常，无法归还书籍");
                return;
            }

            // 归还书籍
            // 根据user_id和book_id找到对应的借阅记录ID
            auto borrow_records = borrow_service->getUserBorrowRecords(user_id, "borrowed", 1, 100); // 增加page_size以获取更多记录
            if (borrow_records.empty()) {
                sendResponse(request, status_codes::BadRequest, 400, "未找到该用户的借阅记录");
                return;
            }

            // 查找与book_id匹配的借阅记录
            int borrow_id = -1;
            for (const auto& record : borrow_records) {
                if (record->getBookId() == book_id) { // 假设BorrowRecord有getBookId()方法
                    borrow_id = record->getId();
                    break;
                }
            }

            if (borrow_id == -1) {
                sendResponse(request, status_codes::BadRequest, 400, "未找到该用户借阅该书籍的记录");
                return;
            }

            bool success = borrow_service->returnBook(borrow_id);

            if (success) {
                sendResponse(request, status_codes::OK, 200, "归还书籍成功");
            } else {
                sendResponse(request, status_codes::InternalError, 500, "归还书籍失败");
            }
        } catch (const exception& e) {
            sendResponse(request, status_codes::InternalError, 500, "归还书籍失败: " + string(e.what()));
        }
    }).wait();
}

void BookController::handleReserveBook(http_request request) {
    // 解析请求体
    request.extract_json().then([=](web::json::value body) {
        try {
            // 验证请求参数
            if (!body.has_field("user_id") || !body.has_field("book_id")) {
                sendResponse(request, status_codes::BadRequest, 400, "缺少必填参数");
                return;
            }

            int user_id = body["user_id"].as_integer();
            int book_id = body["book_id"].as_integer();

            // 验证用户是否存在且状态正常
            shared_ptr<User> user = user_service->getUserInfo(user_id);
            if (!user) {
                sendResponse(request, status_codes::BadRequest, 400, "用户不存在");
                return;
            }

            if (user->getStatus() != "active") {
                sendResponse(request, status_codes::BadRequest, 400, "用户状态异常，无法预约书籍");
                return;
            }

            // 预约书籍
            bool success = reservation_service->reserveBook(user_id, book_id);

            if (success) {
                sendResponse(request, status_codes::OK, 200, "预约书籍成功");
            } else {
                sendResponse(request, status_codes::InternalError, 500, "预约书籍失败");
            }
        } catch (const exception& e) {
            sendResponse(request, status_codes::InternalError, 500, "预约书籍失败: " + string(e.what()));
        }
    }).wait();
}

web::json::value BookController::serializeBook(const shared_ptr<Book>& book) {
    web::json::value json_book = web::json::value::object();
    json_book["id"] = web::json::value::number(book->getId());
    json_book["title"] = web::json::value::string(book->getTitle());
    json_book["author"] = web::json::value::string(book->getAuthor());
    // Book类中没有publisher和publish_date属性，移除相关代码
    json_book["isbn"] = web::json::value::string(book->getIsbn());
    json_book["categories"] = web::json::value::array();

    const vector<string>& categories = book->getCategories();
    for (size_t i = 0; i < categories.size(); ++i) {
        json_book["categories"][i] = web::json::value::string(categories[i]);
    }

    json_book["total_count"] = web::json::value::number(book->getTotalCopies());
    json_book["available_count"] = web::json::value::number(book->getAvailableCopies());
    json_book["status"] = web::json::value::string(book->getStatus());

    return json_book;
}

void BookController::sendResponse(http_request request, status_code status, int code, const string& message, web::json::value data) {
    web::json::value response = web::json::value::object();
    response["code"] = web::json::value::number(code);
    response["message"] = web::json::value::string(message);

    if (!data.is_null()) {
        response["data"] = data;
    }

    request.reply(status, response);
}