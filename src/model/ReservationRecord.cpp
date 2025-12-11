#include "ReservationRecord.h"

ReservationRecord::ReservationRecord(int id, int user_id, int book_id, const std::string& reservation_date, const std::string& status,
                                      const std::string& confirmed_date, const std::string& expire_date, int queue_position,
                                      const std::string& created_at, const std::string& updated_at) {
    id_ = id;
    user_id_ = user_id;
    book_id_ = book_id;
    reservation_date_ = reservation_date;
    status_ = status;
    confirmed_date_ = confirmed_date;
    expire_date_ = expire_date;
    queue_position_ = queue_position;
    created_at_ = created_at;
    updated_at_ = updated_at;
}

int ReservationRecord::getId() const {
    return id_;
}

void ReservationRecord::setId(int id) {
    id_ = id;
}

int ReservationRecord::getUserId() const {
    return user_id_;
}

void ReservationRecord::setUserId(int user_id) {
    user_id_ = user_id;
}

int ReservationRecord::getBookId() const {
    return book_id_;
}

void ReservationRecord::setBookId(int book_id) {
    book_id_ = book_id;
}

std::string ReservationRecord::getReservationDate() const {
    return reservation_date_;
}

void ReservationRecord::setReservationDate(const std::string& reservation_date) {
    reservation_date_ = reservation_date;
}

std::string ReservationRecord::getStatus() const {
    return status_;
}

void ReservationRecord::setStatus(const std::string& status) {
    status_ = status;
}

std::string ReservationRecord::getConfirmedDate() const {
    return confirmed_date_;
}

void ReservationRecord::setConfirmedDate(const std::string& confirmed_date) {
    confirmed_date_ = confirmed_date;
}

std::string ReservationRecord::getExpireDate() const {
    return expire_date_;
}

void ReservationRecord::setExpireDate(const std::string& expire_date) {
    expire_date_ = expire_date;
}

int ReservationRecord::getQueuePosition() const {
    return queue_position_;
}

void ReservationRecord::setQueuePosition(int queue_position) {
    queue_position_ = queue_position;
}

std::string ReservationRecord::getCreatedAt() const {
    return created_at_;
}

void ReservationRecord::setCreatedAt(const std::string& created_at) {
    created_at_ = created_at;
}

std::string ReservationRecord::getUpdatedAt() const {
    return updated_at_;
}

void ReservationRecord::setUpdatedAt(const std::string& updated_at) {
    updated_at_ = updated_at;
}

nlohmann::json ReservationRecord::toJson() const {
    nlohmann::json json_obj;
    json_obj["id"] = id_;
    json_obj["user_id"] = user_id_;
    json_obj["book_id"] = book_id_;
    json_obj["reservation_date"] = reservation_date_;
    json_obj["status"] = status_;
    json_obj["confirmed_date"] = confirmed_date_;
    json_obj["expire_date"] = expire_date_;
    json_obj["queue_position"] = queue_position_;
    json_obj["created_at"] = created_at_;
    json_obj["updated_at"] = updated_at_;
    return json_obj;
}

ReservationRecord ReservationRecord::fromJson(const nlohmann::json& json_obj) {
    ReservationRecord reservation_record;
    if (json_obj.contains("id")) {
        reservation_record.setId(json_obj["id"]);
    }
    if (json_obj.contains("user_id")) {
        reservation_record.setUserId(json_obj["user_id"]);
    }
    if (json_obj.contains("book_id")) {
        reservation_record.setBookId(json_obj["book_id"]);
    }
    if (json_obj.contains("reservation_date")) {
        reservation_record.setReservationDate(json_obj["reservation_date"]);
    }
    if (json_obj.contains("status")) {
        reservation_record.setStatus(json_obj["status"]);
    }
    if (json_obj.contains("confirmed_date")) {
        reservation_record.setConfirmedDate(json_obj["confirmed_date"]);
    }
    if (json_obj.contains("expire_date")) {
        reservation_record.setExpireDate(json_obj["expire_date"]);
    }
    if (json_obj.contains("queue_position")) {
        reservation_record.setQueuePosition(json_obj["queue_position"]);
    }
    if (json_obj.contains("created_at")) {
        reservation_record.setCreatedAt(json_obj["created_at"]);
    }
    if (json_obj.contains("updated_at")) {
        reservation_record.setUpdatedAt(json_obj["updated_at"]);
    }
    return reservation_record;
}