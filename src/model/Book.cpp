#include "Book.h"

Book::Book(int id, const std::string& title, const std::string& author, const std::string& isbn,
           const std::string& description, int total_copies, int available_copies, int borrowed_copies,
           const std::string& status, const std::string& created_at, const std::string& updated_at) {
    id_ = id;
    title_ = title;
    author_ = author;
    isbn_ = isbn;
    description_ = description;
    total_copies_ = total_copies;
    available_copies_ = available_copies;
    borrowed_copies_ = borrowed_copies;
    status_ = status;
    created_at_ = created_at;
    updated_at_ = updated_at;
}

int Book::getId() const {
    return id_;
}

void Book::setId(int id) {
    id_ = id;
}

std::string Book::getTitle() const {
    return title_;
}

void Book::setTitle(const std::string& title) {
    title_ = title;
}

std::string Book::getAuthor() const {
    return author_;
}

void Book::setAuthor(const std::string& author) {
    author_ = author;
}

std::string Book::getIsbn() const {
    return isbn_;
}

void Book::setIsbn(const std::string& isbn) {
    isbn_ = isbn;
}

std::string Book::getDescription() const {
    return description_;
}

void Book::setDescription(const std::string& description) {
    description_ = description;
}

int Book::getTotalCopies() const {
    return total_copies_;
}

void Book::setTotalCopies(int total_copies) {
    total_copies_ = total_copies;
}

int Book::getAvailableCopies() const {
    return available_copies_;
}

void Book::setAvailableCopies(int available_copies) {
    available_copies_ = available_copies;
}

int Book::getBorrowedCopies() const {
    return borrowed_copies_;
}

void Book::setBorrowedCopies(int borrowed_copies) {
    borrowed_copies_ = borrowed_copies;
}

std::string Book::getStatus() const {
    return status_;
}

void Book::setStatus(const std::string& status) {
    status_ = status;
}

std::string Book::getCreatedAt() const {
    return created_at_;
}

void Book::setCreatedAt(const std::string& created_at) {
    created_at_ = created_at;
}

std::string Book::getUpdatedAt() const {
    return updated_at_;
}

void Book::setUpdatedAt(const std::string& updated_at) {
    updated_at_ = updated_at;
}

std::vector<std::string> Book::getCategories() const {
    return categories_;
}

void Book::setCategories(const std::vector<std::string>& categories) {
    categories_ = categories;
}

nlohmann::json Book::toJson() const {
    nlohmann::json json_obj;
    json_obj["id"] = id_;
    json_obj["title"] = title_;
    json_obj["author"] = author_;
    json_obj["isbn"] = isbn_;
    json_obj["description"] = description_;
    json_obj["total_copies"] = total_copies_;
    json_obj["available_copies"] = available_copies_;
    json_obj["borrowed_copies"] = borrowed_copies_;
    json_obj["status"] = status_;
    json_obj["created_at"] = created_at_;
    json_obj["updated_at"] = updated_at_;
    json_obj["categories"] = categories_;
    return json_obj;
}

Book Book::fromJson(const nlohmann::json& json_obj) {
    Book book;
    if (json_obj.contains("id")) {
        book.setId(json_obj["id"]);
    }
    if (json_obj.contains("title")) {
        book.setTitle(json_obj["title"]);
    }
    if (json_obj.contains("author")) {
        book.setAuthor(json_obj["author"]);
    }
    if (json_obj.contains("isbn")) {
        book.setIsbn(json_obj["isbn"]);
    }
    if (json_obj.contains("description")) {
        book.setDescription(json_obj["description"]);
    }
    if (json_obj.contains("total_copies")) {
        book.setTotalCopies(json_obj["total_copies"]);
    }
    if (json_obj.contains("available_copies")) {
        book.setAvailableCopies(json_obj["available_copies"]);
    }
    if (json_obj.contains("borrowed_copies")) {
        book.setBorrowedCopies(json_obj["borrowed_copies"]);
    }
    if (json_obj.contains("status")) {
        book.setStatus(json_obj["status"]);
    }
    if (json_obj.contains("created_at")) {
        book.setCreatedAt(json_obj["created_at"]);
    }
    if (json_obj.contains("updated_at")) {
        book.setUpdatedAt(json_obj["updated_at"]);
    }
    if (json_obj.contains("categories")) {
        std::vector<std::string> categories = json_obj["categories"];
        book.setCategories(categories);
    }
    return book;
}