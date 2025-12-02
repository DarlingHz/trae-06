#include "BorrowRecord.h"

BorrowRecord::BorrowRecord(int id, int user_id, int book_id, const std::string& borrow_date, const std::string& due_date,
                             const std::string& return_date, const std::string& status, const std::string& created_at,
                             const std::string& updated_at) {
    id_ = id;
    user_id_ = user_id;
    book_id_ = book_id;
    borrow_date_ = borrow_date;
    due_date_ = due_date;
    return_date_ = return_date;
    status_ = status;
    created_at_ = created_at;
    updated_at_ = updated_at;
}

int BorrowRecord::getId() const {
    return id_;
}

void BorrowRecord::setId(int id) {
    id_ = id;
}

int BorrowRecord::getUserId() const {
    return user_id_;
}

void BorrowRecord::setUserId(int user_id) {
    user_id_ = user_id;
}

int BorrowRecord::getBookId() const {
    return book_id_;
}

void BorrowRecord::setBookId(int book_id) {
    book_id_ = book_id;
}

std::string BorrowRecord::getBorrowDate() const {
    return borrow_date_;
}

void BorrowRecord::setBorrowDate(const std::string& borrow_date) {
    borrow_date_ = borrow_date;
}

std::string BorrowRecord::getDueDate() const {
    return due_date_;
}

void BorrowRecord::setDueDate(const std::string& due_date) {
    due_date_ = due_date;
}

std::string BorrowRecord::getReturnDate() const {
    return return_date_;
}

void BorrowRecord::setReturnDate(const std::string& return_date) {
    return_date_ = return_date;
}

std::string BorrowRecord::getStatus() const {
    return status_;
}

void BorrowRecord::setStatus(const std::string& status) {
    status_ = status;
}

std::string BorrowRecord::getCreatedAt() const {
    return created_at_;
}

void BorrowRecord::setCreatedAt(const std::string& created_at) {
    created_at_ = created_at;
}

std::string BorrowRecord::getUpdatedAt() const {
    return updated_at_;
}

void BorrowRecord::setUpdatedAt(const std::string& updated_at) {
    updated_at_ = updated_at;
}

nlohmann::json BorrowRecord::toJson() const {
    nlohmann::json json_obj;
    json_obj["id"] = id_;
    json_obj["user_id"] = user_id_;
    json_obj["book_id"] = book_id_;
    json_obj["borrow_date"] = borrow_date_;
    json_obj["due_date"] = due_date_;
    json_obj["return_date"] = return_date_;
    json_obj["status"] = status_;
    json_obj["created_at"] = created_at_;
    json_obj["updated_at"] = updated_at_;
    return json_obj;
}

BorrowRecord BorrowRecord::fromJson(const nlohmann::json& json_obj) {
    BorrowRecord borrow_record;
    if (json_obj.contains("id")) {
        borrow_record.setId(json_obj["id"]);
    }
    if (json_obj.contains("user_id")) {
        borrow_record.setUserId(json_obj["user_id"]);
    }
    if (json_obj.contains("book_id")) {
        borrow_record.setBookId(json_obj["book_id"]);
    }
    if (json_obj.contains("borrow_date")) {
        borrow_record.setBorrowDate(json_obj["borrow_date"]);
    }
    if (json_obj.contains("due_date")) {
        borrow_record.setDueDate(json_obj["due_date"]);
    }
    if (json_obj.contains("return_date")) {
        borrow_record.setReturnDate(json_obj["return_date"]);
    }
    if (json_obj.contains("status")) {
        borrow_record.setStatus(json_obj["status"]);
    }
    if (json_obj.contains("created_at")) {
        borrow_record.setCreatedAt(json_obj["created_at"]);
    }
    if (json_obj.contains("updated_at")) {
        borrow_record.setUpdatedAt(json_obj["updated_at"]);
    }
    return borrow_record;
}