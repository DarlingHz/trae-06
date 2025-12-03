#include "parking/dao.h"
#include "parking/database.h"
#include "parking/models.h"
#include <stdexcept>
#include <vector>

// SQLiteReservationDAO实现
int SQLiteReservationDAO::create(const Reservation& reservation) {
    try {
        // 使用事务确保一致性
        db::get().begin_transaction();

        // 首先检查是否有冲突（防止并发问题）
        if (has_conflict(reservation.spot_id, reservation.start_time, reservation.end_time)) {
            db::get().rollback();
            throw std::runtime_error("Reservation conflict detected");
        }

        db::get().execute(
            "INSERT INTO reservations (spot_id, renter_user_id, vehicle_plate, start_time, end_time, "
            "total_price, status, created_at, updated_at) "
            "VALUES (" 
            + std::to_string(reservation.spot_id) + ", "
            + std::to_string(reservation.renter_user_id) + ", "
            "'" + reservation.vehicle_plate + "', "
            + std::to_string(reservation.start_time) + ", "
            + std::to_string(reservation.end_time) + ", "
            + std::to_string(reservation.total_price) + ", "
            "'" + to_string(reservation.status) + "', "
            + std::to_string(reservation.created_at) + ", "
            + std::to_string(reservation.updated_at) + ")"
        );

        int reservation_id = static_cast<int>(db::get().last_insert_rowid());
        db::get().commit();

        return reservation_id;
    } catch (const std::exception& e) {
        try {
            db::get().rollback();
        } catch (...) {
            // 忽略回滚错误
        }
        throw std::runtime_error("Failed to create reservation: " + std::string(e.what()));
    }
}

std::optional<Reservation> SQLiteReservationDAO::find_by_id(int id) const {
    Reservation reservation;
    bool found = false;

    try {
        db::get().query(
            "SELECT id, spot_id, renter_user_id, vehicle_plate, start_time, end_time, "
            "total_price, status, created_at, updated_at "
            "FROM reservations WHERE id = " + std::to_string(id),
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 10) {
                    reservation.id = std::stoi(argv[0]);
                    reservation.spot_id = std::stoi(argv[1]);
                    reservation.renter_user_id = std::stoi(argv[2]);
                    reservation.vehicle_plate = argv[3];
                    reservation.start_time = std::stoll(argv[4]);
                    reservation.end_time = std::stoll(argv[5]);
                    reservation.total_price = std::stod(argv[6]);
                    reservation.status = reservation_status_from_string(argv[7]);
                    reservation.created_at = std::stoll(argv[8]);
                    reservation.updated_at = std::stoll(argv[9]);
                    found = true;
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find reservation by id: " + std::string(e.what()));
    }

    return found ? std::optional<Reservation>(reservation) : std::nullopt;
}

std::vector<Reservation> SQLiteReservationDAO::find_by_renter(int renter_id) const {
    std::vector<Reservation> reservations;

    try {
        db::get().query(
            "SELECT id, spot_id, renter_user_id, vehicle_plate, start_time, end_time, "
            "total_price, status, created_at, updated_at "
            "FROM reservations WHERE renter_user_id = " + std::to_string(renter_id),
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 10) {
                    Reservation res;
                    res.id = std::stoi(argv[0]);
                    res.spot_id = std::stoi(argv[1]);
                    res.renter_user_id = std::stoi(argv[2]);
                    res.vehicle_plate = argv[3];
                    res.start_time = std::stoll(argv[4]);
                    res.end_time = std::stoll(argv[5]);
                    res.total_price = std::stod(argv[6]);
                    res.status = reservation_status_from_string(argv[7]);
                    res.created_at = std::stoll(argv[8]);
                    res.updated_at = std::stoll(argv[9]);
                    reservations.push_back(res);
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find reservations by renter: " + std::string(e.what()));
    }

    return reservations;
}

std::vector<Reservation> SQLiteReservationDAO::find_by_spot(int spot_id) const {
    std::vector<Reservation> reservations;

    try {
        db::get().query(
            "SELECT id, spot_id, renter_user_id, vehicle_plate, start_time, end_time, "
            "total_price, status, created_at, updated_at "
            "FROM reservations WHERE spot_id = " + std::to_string(spot_id),
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 10) {
                    Reservation res;
                    res.id = std::stoi(argv[0]);
                    res.spot_id = std::stoi(argv[1]);
                    res.renter_user_id = std::stoi(argv[2]);
                    res.vehicle_plate = argv[3];
                    res.start_time = std::stoll(argv[4]);
                    res.end_time = std::stoll(argv[5]);
                    res.total_price = std::stod(argv[6]);
                    res.status = reservation_status_from_string(argv[7]);
                    res.created_at = std::stoll(argv[8]);
                    res.updated_at = std::stoll(argv[9]);
                    reservations.push_back(res);
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find reservations by spot: " + std::string(e.what()));
    }

    return reservations;
}

std::vector<Reservation> SQLiteReservationDAO::find_by_owner(int owner_id) const {
    std::vector<Reservation> reservations;

    try {
        db::get().query(
            "SELECT r.id, r.spot_id, r.renter_user_id, r.vehicle_plate, r.start_time, r.end_time, "
            "r.total_price, r.status, r.created_at, r.updated_at "
            "FROM reservations r JOIN parking_spots ps ON r.spot_id = ps.id "
            "WHERE ps.owner_user_id = " + std::to_string(owner_id),
            [&](int argc, char** argv, char** azColName) -> int {
                if (argc >= 10) {
                    Reservation res;
                    res.id = std::stoi(argv[0]);
                    res.spot_id = std::stoi(argv[1]);
                    res.renter_user_id = std::stoi(argv[2]);
                    res.vehicle_plate = argv[3];
                    res.start_time = std::stoll(argv[4]);
                    res.end_time = std::stoll(argv[5]);
                    res.total_price = std::stod(argv[6]);
                    res.status = reservation_status_from_string(argv[7]);
                    res.created_at = std::stoll(argv[8]);
                    res.updated_at = std::stoll(argv[9]);
                    reservations.push_back(res);
                }
                return 0;
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to find reservations by owner: " + std::string(e.what()));
    }

    return reservations;
}

bool SQLiteReservationDAO::update_status(int id, ReservationStatus status) {
    try {
        db::get().execute(
            "UPDATE reservations SET "
            "status = '" + to_string(status) + "', "
            "updated_at = " + std::to_string(std::time(nullptr)) + " "
            "WHERE id = " + std::to_string(id)
        );
        return sqlite3_changes(db::get().get_native_handle()) > 0;
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to update reservation status: " + std::string(e.what()));
    }
}

bool SQLiteReservationDAO::has_conflict(int spot_id, std::time_t start_time, std::time_t end_time) const {
    bool has_conflict = false;

    try {
        db::get().query(
            "SELECT 1 FROM reservations "
            "WHERE spot_id = " + std::to_string(spot_id) + " "
            "AND status IN ('pending', 'confirmed') "
            "AND start_time < " + std::to_string(end_time) + " "
            "AND end_time > " + std::to_string(start_time),
            [&](int argc, char** argv, char** azColName) -> int {
                has_conflict = true;
                return 1; // 停止查询
            }
        );
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to check reservation conflict: " + std::string(e.what()));
    }

    return has_conflict;
}
