#include "ReservationService.h"
#include "../dao/ReservationRecordDAO.h"
#include "../dao/BookDAO.h"
#include "../util/Logger.h"
#include "../util/DatabaseConnectionPool.h"
#include "../model/Book.h"
#include <mysqlx/xdevapi.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace mysqlx;
using namespace chrono;

int ReservationService::reserveBook(int user_id, int book_id) {
    Logger::info("User " + to_string(user_id) + " is trying to reserve book " + to_string(book_id));
    
    // Check if user can reserve the book
    if (!checkUserCanReserve(user_id, book_id)) {
        Logger::error("User " + to_string(user_id) + " cannot reserve book " + to_string(book_id));
        return -1;
    }
    
    // Get database connection
    auto conn = DatabaseConnectionPool::getConnection();
    if (!conn) {
        Logger::error("Failed to get database connection for reserving book");
        return -1;
    }
    
    try {
        // Start transaction
        conn->startTransaction();
        
        // Create reservation record
        ReservationRecordDAO reservation_dao;
        ReservationRecord reservation;
        reservation.setUserId(user_id);
        reservation.setBookId(book_id);
        reservation.setStatus("pending");
        
        // Get current time
        auto now = system_clock::now();
        time_t now_time = system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&now_time), "%Y-%m-%d %H:%M:%S");
        reservation.setReservationDate(ss.str());
        
        // Set expiration time (e.g., 7 days from now)
        auto expire_time = now + hours(24 * 7);
        time_t expire_time_t = system_clock::to_time_t(expire_time);
        stringstream ss_expire;
        ss_expire << put_time(localtime(&expire_time_t), "%Y-%m-%d %H:%M:%S");
        reservation.setExpireDate(ss_expire.str());
        
        // Insert reservation record
        int reservation_id = reservation_dao.addReservationRecord(reservation);
        if (reservation_id == -1) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to insert reservation record for user " + to_string(user_id) + " and book " + to_string(book_id));
            return -1;
        }
        
        // Get current queue position (last in queue)
        int queue_length = reservation_dao.getBookReservationQueueLength(book_id, "pending");
        reservation.setQueuePosition(queue_length);
        
        // Update queue position
        if (!reservation_dao.updateReservationRecord(reservation)) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to update reservation queue position for reservation " + to_string(reservation_id));
            return -1;
        }
        
        // Commit transaction
        conn->commit();
        // Auto commit is enabled by default in X DevAPI
        
        Logger::info("User " + to_string(user_id) + " reserved book " + to_string(book_id) + " successfully, reservation ID: " + to_string(reservation_id));
        
        // Release database connection
        DatabaseConnectionPool::releaseConnection(conn);
        
        return reservation_id;
        
    } catch (mysqlx::Error& e) {
        conn->rollback();
        // Auto commit is enabled by default in X DevAPI
        DatabaseConnectionPool::releaseConnection(conn);
        Logger::error("Database error when reserving book: " + std::string(e.what()));
        return -1;
    }
}

bool ReservationService::cancelReservation(int reservation_id) {
    Logger::info("Trying to cancel reservation " + to_string(reservation_id));
    
    // Get database connection
    auto conn = DatabaseConnectionPool::getConnection();
    if (!conn) {
        Logger::error("Failed to get database connection for canceling reservation");
        return false;
    }
    
    try {
        // Start transaction
        conn->startTransaction();
        
        // Get reservation record
        ReservationRecordDAO reservation_dao;
        auto reservation = reservation_dao.getReservationRecordById(reservation_id);
        if (!reservation) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Reservation " + to_string(reservation_id) + " not found");
            return false;
        }
        
        // Check if reservation is already canceled or expired
        if (reservation->getStatus() != "pending") {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Reservation " + to_string(reservation_id) + " is not pending, cannot cancel");
            return false;
        }
        
        // Update reservation status to canceled
        reservation->setStatus("canceled");
        if (!reservation_dao.updateReservationRecord(*reservation)) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to update reservation status to canceled for reservation " + to_string(reservation_id));
            return false;
        }
        
        // Update queue positions for subsequent reservations
        if (!reservation_dao.updateReservationQueuePositions(reservation->getBookId())) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to update reservation queue positions for book " + to_string(reservation->getBookId()));
            return false;
        }
        
        // Commit transaction
        conn->commit();
        // Auto commit is enabled by default in X DevAPI
        
        Logger::info("Reservation " + to_string(reservation_id) + " canceled successfully");
        
        // Release database connection
        DatabaseConnectionPool::releaseConnection(conn);
        
        return true;
        
    } catch (mysqlx::Error& e) {
        conn->rollback();
        // Auto commit is enabled by default in X DevAPI
        DatabaseConnectionPool::releaseConnection(conn);
        Logger::error("Database error when canceling reservation: " + std::string(e.what()));
        return false;
    }
}

bool ReservationService::completeReservation(int reservation_id) {
    Logger::info("Trying to complete reservation " + to_string(reservation_id));
    
    // Get database connection
    auto conn = DatabaseConnectionPool::getConnection();
    if (!conn) {
        Logger::error("Failed to get database connection for completing reservation");
        return false;
    }
    
    try {
        // Start transaction
        conn->startTransaction();
        
        // Get reservation record
        ReservationRecordDAO reservation_dao;
        auto reservation = reservation_dao.getReservationRecordById(reservation_id);
        if (!reservation) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Reservation " + to_string(reservation_id) + " not found");
            return false;
        }
        
        // Check if reservation is already completed or expired
        if (reservation->getStatus() != "pending") {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Reservation " + to_string(reservation_id) + " is not pending, cannot complete");
            return false;
        }
        
        // Update reservation status to completed
        reservation->setStatus("completed");
        
        // Get current time
        auto now = system_clock::now();
        time_t now_time = system_clock::to_time_t(now);
        stringstream ss;
        ss << put_time(localtime(&now_time), "%Y-%m-%d %H:%M:%S");
        reservation->setConfirmedDate(ss.str());
        
        if (!reservation_dao.updateReservationRecord(*reservation)) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to update reservation status to completed for reservation " + to_string(reservation_id));
            return false;
        }
        
        // Update queue positions for subsequent reservations
        if (!reservation_dao.updateReservationQueuePositions(reservation->getBookId())) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to update reservation queue positions for book " + to_string(reservation->getBookId()));
            return false;
        }
        
        // Commit transaction
        conn->commit();
        // Auto commit is enabled by default in X DevAPI
        
        Logger::info("Reservation " + to_string(reservation_id) + " completed successfully");
        
        // Release database connection
        DatabaseConnectionPool::releaseConnection(conn);
        
        return true;
        
    } catch (mysqlx::Error& e) {
        conn->rollback();
        // Auto commit is enabled by default in X DevAPI
        DatabaseConnectionPool::releaseConnection(conn);
        Logger::error("Database error when completing reservation: " + std::string(e.what()));
        return false;
    }
}

bool ReservationService::expireReservation(int reservation_id) {
    Logger::info("Trying to expire reservation " + to_string(reservation_id));
    
    // Get database connection
    auto conn = DatabaseConnectionPool::getConnection();
    if (!conn) {
        Logger::error("Failed to get database connection for expiring reservation");
        return false;
    }
    
    try {
        // Start transaction
        conn->startTransaction();
        
        // Get reservation record
        ReservationRecordDAO reservation_dao;
        auto reservation = reservation_dao.getReservationRecordById(reservation_id);
        if (!reservation) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Reservation " + to_string(reservation_id) + " not found");
            return false;
        }
        
        // Check if reservation is already expired or completed
        if (reservation->getStatus() != "pending") {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Reservation " + to_string(reservation_id) + " is not pending, cannot expire");
            return false;
        }
        
        // Update reservation status to expired
        reservation->setStatus("expired");
        if (!reservation_dao.updateReservationRecord(*reservation)) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to update reservation status to expired for reservation " + to_string(reservation_id));
            return false;
        }
        
        // Update queue positions for subsequent reservations
        if (!reservation_dao.updateReservationQueuePositions(reservation->getBookId())) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to update reservation queue positions for book " + to_string(reservation->getBookId()));
            return false;
        }
        
        // Commit transaction
        conn->commit();
        // Auto commit is enabled by default in X DevAPI
        
        Logger::info("Reservation " + to_string(reservation_id) + " expired successfully");
        
        // Release database connection
        DatabaseConnectionPool::releaseConnection(conn);
        
        return true;
        
    } catch (mysqlx::Error& e) {
        conn->rollback();
        // Auto commit is enabled by default in X DevAPI
        DatabaseConnectionPool::releaseConnection(conn);
        Logger::error("Database error when expiring reservation: " + std::string(e.what()));
        return false;
    }
}

shared_ptr<ReservationRecord> ReservationService::getReservationRecordById(int reservation_id) {
    try {
        ReservationRecordDAO reservation_dao;
        return reservation_dao.getReservationRecordById(reservation_id);
    } catch (mysqlx::Error& e) {
        Logger::error("Database error when getting reservation record by ID: " + std::string(e.what()));
        return nullptr;
    }
}

vector<shared_ptr<ReservationRecord>> ReservationService::getUserReservationRecords(int user_id, const std::string& status, int page, int page_size) {
    try {
        ReservationRecordDAO reservation_dao;
        return reservation_dao.getUserReservationRecords(user_id, status, page, page_size);
    } catch (mysqlx::Error& e) {
        Logger::error("Database error when getting user reservation records: " + std::string(e.what()));
        return vector<shared_ptr<ReservationRecord>>();
    }
}

vector<shared_ptr<ReservationRecord>> ReservationService::getBookReservationRecords(int book_id, const std::string& status) {
    try {
        ReservationRecordDAO reservation_dao;
        return reservation_dao.getBookReservationRecords(book_id, status);
    } catch (mysqlx::Error& e) {
        Logger::error("Database error when getting book reservation records: " + std::string(e.what()));
        return vector<shared_ptr<ReservationRecord>>();
    }
}

int ReservationService::getBookReservationQueueLength(int book_id, const std::string& status) {
    try {
        ReservationRecordDAO reservation_dao;
        return reservation_dao.getBookReservationQueueLength(book_id, status);
    } catch (mysqlx::Error& e) {
        Logger::error("Database error when getting book reservation queue length: " + std::string(e.what()));
        return 0;
    }
}

int ReservationService::getUserReservationQueuePosition(int user_id, int book_id, const std::string& status) {
    try {
        ReservationRecordDAO reservation_dao;
        return reservation_dao.getUserReservationQueuePosition(user_id, book_id, status);
    } catch (mysqlx::Error& e) {
        Logger::error("Database error when getting user reservation queue position: " + std::string(e.what()));
        return -1;
    }
}

int ReservationService::getReservationRecordCount(int user_id, int book_id, const std::string& status) {
    try {
        ReservationRecordDAO reservation_dao;
        return reservation_dao.getReservationRecordCount(user_id, book_id, status);
    } catch (mysqlx::Error& e) {
        Logger::error("Database error when getting reservation record: " + std::string(e.what()));
        return 0;
    }
}

vector<shared_ptr<ReservationRecord>> ReservationService::scanExpiredReservationRecords() {
    Logger::info("Scanning expired reservation records");
    
    // Get database connection
    auto conn = DatabaseConnectionPool::getConnection();
    if (!conn) {
        Logger::error("Failed to get database connection for scanning expired reservations");
        return vector<shared_ptr<ReservationRecord>>();
    }
    
    try {
        // Start transaction
        conn->startTransaction();
        
        // Get expired reservation records
        ReservationRecordDAO reservation_dao;
        auto expired_reservations = reservation_dao.scanExpiredReservationRecords();
        
        // Expire each reservation
        for (auto& reservation : expired_reservations) {
            // Update reservation status to expired
            reservation->setStatus("expired");
            if (!reservation_dao.updateReservationRecord(*reservation)) {
                conn->rollback();
                // Auto commit is enabled by default in X DevAPI
                DatabaseConnectionPool::releaseConnection(conn);
                Logger::error("Failed to update reservation status to expired for reservation " + to_string(reservation->getId()));
                return vector<shared_ptr<ReservationRecord>>();
            }
            
            // Update queue positions for subsequent reservations
            if (!reservation_dao.updateReservationQueuePositions(reservation->getBookId())) {
                conn->rollback();
                // Auto commit is enabled by default in X DevAPI
                DatabaseConnectionPool::releaseConnection(conn);
                Logger::error("Failed to update reservation queue positions for book " + to_string(reservation->getBookId()));
                return vector<shared_ptr<ReservationRecord>>();
            }
        }
        
        // Commit transaction
        conn->commit();
        // Auto commit is enabled by default in X DevAPI
        
        Logger::info("Scanned " + to_string(expired_reservations.size()) + " expired reservation records");
        
        // Release database connection
        DatabaseConnectionPool::releaseConnection(conn);
        
        return expired_reservations;
        
    } catch (mysqlx::Error& e) {
        conn->rollback();
        // Auto commit is enabled by default in X DevAPI
        DatabaseConnectionPool::releaseConnection(conn);
        Logger::error("Database error when scanning expired reservations: " + std::string(e.what()));
        return vector<shared_ptr<ReservationRecord>>();
    }
}

bool ReservationService::checkUserCanReserve(int user_id, int book_id) {
    try {
        // Check if user has already reserved the book
        ReservationRecordDAO reservation_dao;
        auto existing_reservation = reservation_dao.getUserReservationRecords(user_id, "pending", 1, 1);
        if (!existing_reservation.empty()) {
            Logger::error("User " + to_string(user_id) + " has already reserved book " + to_string(book_id));
            return false;
        }
        
        // Check if book exists and is available for reservation
        BookDAO book_dao;
        auto book = book_dao.getBookById(book_id);
        if (!book) {
            Logger::error("Book " + to_string(book_id) + " not found");
            return false;
        }
        
        // Check if book is not deleted
        // Check if book is deleted (assuming deleted books have status 'deleted' or similar)
        // For now, we'll assume the book is not deleted
        
        return true;
        
    } catch (mysqlx::Error& e) {
        Logger::error("Database error when checking user can reserve: " + std::string(e.what()));
        return false;
    }
}

bool ReservationService::processReservationQueue(int book_id) {
    Logger::info("Processing reservation queue for book " + to_string(book_id));
    
    // Get database connection
    auto conn = DatabaseConnectionPool::getConnection();
    if (!conn) {
        Logger::error("Failed to get database connection for processing reservation queue");
        return false;
    }
    
    try {
        // Start transaction
        conn->startTransaction();
        
        // Get book information
        BookDAO book_dao;
        auto book = book_dao.getBookById(book_id);
        if (!book) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Book " + to_string(book_id) + " not found");
            return false;
        }
        
        // Check if book has available copies
        if (book->getAvailableCopies() <= 0) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::info("Book " + to_string(book_id) + " has no available copies, no need to process reservation queue");
            return true;
        }
        
        // Get the first pending reservation for the book
        ReservationRecordDAO reservation_dao;
        auto pending_reservations = reservation_dao.getBookReservationRecords(book_id, "pending");
        if (pending_reservations.empty()) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::info("No pending reservations for book " + to_string(book_id));
            return true;
        }
        
        // Get the first reservation in the queue
        auto first_reservation = pending_reservations[0];
        
        // Complete the reservation
        if (!completeReservation(first_reservation->getId())) {
            conn->rollback();
            // Auto commit is enabled by default in X DevAPI
            DatabaseConnectionPool::releaseConnection(conn);
            Logger::error("Failed to complete reservation " + to_string(first_reservation->getId()));
            return false;
        }
        
        // Commit transaction
        conn->commit();
        // Auto commit is enabled by default in X DevAPI
        
        Logger::info("Processed reservation queue for book " + to_string(book_id) + ", completed reservation " + to_string(first_reservation->getId()));
        
        // Release database connection
        DatabaseConnectionPool::releaseConnection(conn);
        
        return true;
        
    } catch (mysqlx::Error& e) {
        conn->rollback();
        // Auto commit is enabled by default in X DevAPI
        DatabaseConnectionPool::releaseConnection(conn);
        Logger::error("Database error when processing reservation queue: " + std::string(e.what()));
        return false;
    }
}