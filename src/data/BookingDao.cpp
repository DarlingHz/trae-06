#include "BookingDao.hpp"
#include "../util/Logger.hpp"
#include "MemberDao.hpp"
#include "ClassSessionDao.hpp"
#include <stdexcept>

oatpp::Object<BookingDto> BookingDao::createBooking(const oatpp::Object<BookingDto>& bookingDto) {
  try {
    // 检查会员是否存在
    MemberDao memberDao(getExecutor());
    auto member = memberDao.getMemberById(bookingDto->member_id);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    // 检查课节是否存在
    ClassSessionDao sessionDao(getExecutor());
    auto session = sessionDao.getClassSessionById(bookingDto->session_id);
    if (!session) {
      throw std::runtime_error("Class session not found");
    }

    // 检查课节是否已预约满
    if (session->booked_count >= session->capacity) {
      throw std::runtime_error("Class session is full");
    }

    // 检查会员是否已经预约了该课节
    if (isMemberBooked(bookingDto->member_id, bookingDto->session_id)) {
      throw std::runtime_error("Member already booked this class session");
    }

    // 开始事务
    auto connection = this->getConnection();
    auto transaction = this->beginTransaction(connection);

    try {
      // 创建预约DTO
      auto newBookingDto = oatpp::Object<BookingDto>::createShared();
      newBookingDto->member_id = bookingDto->member_id;
      newBookingDto->session_id = bookingDto->session_id;
      newBookingDto->status = "booked";
      newBookingDto->created_at = bookingDto->created_at;

      // 执行插入操作
      std::unordered_map<oatpp::String, oatpp::Void> insertParams;
      insertParams["member_id"] = newBookingDto->member_id;
      insertParams["session_id"] = newBookingDto->session_id;
      insertParams["status"] = newBookingDto->status;
      insertParams["created_at"] = newBookingDto->created_at;
      auto result = executeQuery(SQL_CREATE_BOOKING, insertParams, connection);

      if (!result->isSuccess()) {
        throw std::runtime_error("Failed to create booking: " + result->getErrorMessage());
      }

      // 获取插入的ID
      auto idResult = executeQuery("SELECT last_insert_rowid() as id", {}, connection);
      auto idRows = idResult->fetch<oatpp::Vector<oatpp::Fields<oatpp::Any>>>();
      if (!idRows || idRows->empty()) {
        throw std::runtime_error("Failed to retrieve inserted booking ID");
      }
      oatpp::Any idValue = idRows->at(0)["id"];
      auto idInt64 = idValue.template retrieve<oatpp::Int64>();
      oatpp::Int32 newId = oatpp::Int32(static_cast<v_int32>(idInt64));

      // 更新课节的预约人数
      if (!sessionDao.updateBookedCount(bookingDto->session_id, 1, connection)) {
        throw std::runtime_error("Failed to update booked count");
      }

      // 提交事务
      transaction.commit();

      Logger::info("Booking created successfully with ID: %d", static_cast<int>(newId));

      // 返回新创建的预约
      return getBookingById(newId);

    } catch (const std::exception& e) {
      // 回滚事务
      transaction.rollback();
      throw;
    }

  } catch (const std::exception& e) {
    Logger::error("Failed to create booking: %s", e.what());
    throw;
  }
}

oatpp::Object<BookingDto> BookingDao::getBookingById(const oatpp::Int32& id) {
  try {
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["id"] = id;
    auto result = executeQuery(SQL_GET_BOOKING_BY_ID, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get booking by ID: " + result->getErrorMessage());
    }

    auto bookings = result->fetch<oatpp::Vector<oatpp::Object<BookingDto>>>();
    if (bookings->size() > 0) {
      return bookings->at(0);
    }

    return nullptr;

  } catch (const std::exception& e) {
    Logger::error("Failed to get booking by ID: %s", e.what());
    throw;
  }
}

oatpp::List<oatpp::Object<BookingDto>> BookingDao::getMemberBookings(
  const oatpp::Int32& memberId,
  const oatpp::String& status,
  const oatpp::Boolean& upcoming
) {
  try {
    // 检查会员是否存在
    MemberDao memberDao(getExecutor());
    auto member = memberDao.getMemberById(memberId);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    std::unordered_map<oatpp::String, oatpp::Void> memberParams;
    memberParams["member_id"] = memberId;
    memberParams["status"] = status;
    memberParams["upcoming"] = upcoming;
    auto result = executeQuery(SQL_GET_MEMBER_BOOKINGS, memberParams);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get member bookings: " + result->getErrorMessage());
    }

    auto bookings = result->fetch<oatpp::Vector<oatpp::Object<BookingDto>>>();
    auto bookingList = oatpp::List<oatpp::Object<BookingDto>>::createShared();
    for (auto& bookingDto : *bookings) {
      bookingList->push_back(bookingDto);
    }

    return bookingList;

  } catch (const std::exception& e) {
    Logger::error("Failed to get member bookings: %s", e.what());
    throw;
  }
}

oatpp::List<oatpp::Object<BookingDto>> BookingDao::getSessionBookings(const oatpp::Int32& sessionId) {
  try {
    // 检查课节是否存在
    ClassSessionDao sessionDao(getExecutor());
    auto session = sessionDao.getClassSessionById(sessionId);
    if (!session) {
      throw std::runtime_error("Class session not found");
    }

    std::unordered_map<oatpp::String, oatpp::Void> sessionParams;
    sessionParams["session_id"] = sessionId;
    auto result = executeQuery(SQL_GET_SESSION_BOOKINGS, sessionParams);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get session bookings: " + result->getErrorMessage());
    }

    auto bookings = result->fetch<oatpp::Vector<oatpp::Object<BookingDto>>>();
    auto bookingList = oatpp::List<oatpp::Object<BookingDto>>::createShared();
    for (auto& bookingDto : *bookings) {
      bookingList->push_back(bookingDto);
    }

    return bookingList;

  } catch (const std::exception& e) {
    Logger::error("Failed to get session bookings: %s", e.what());
    throw;
  }
}

bool BookingDao::isMemberBooked(const oatpp::Int32& memberId, const oatpp::Int32& sessionId) {
  try {
    std::unordered_map<oatpp::String, oatpp::Void> isBookedParams;
    isBookedParams["member_id"] = memberId;
    isBookedParams["session_id"] = sessionId;
    auto result = executeQuery(SQL_IS_MEMBER_BOOKED, isBookedParams);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to check if member is booked: " + result->getErrorMessage());
    }

    auto countResult = result->fetch<oatpp::Vector<oatpp::Fields<oatpp::Any>>>();
    if (countResult->size() > 0) {
      oatpp::Any countValue = countResult->at(0)["COUNT(*)"];
      auto countInt64 = countValue.template retrieve<oatpp::Int64>();
      oatpp::Int32 count = oatpp::Int32(static_cast<v_int32>(countInt64));
      return count > 0;
    }

    return false;

  } catch (const std::exception& e) {
    Logger::error("Failed to check if member is booked: %s", e.what());
    throw;
  }
}

oatpp::Object<BookingDto> BookingDao::updateBookingStatus(const oatpp::Int32& id, const oatpp::String& status) {
  try {
    // 检查预约是否存在
    auto existingBooking = getBookingById(id);
    if (!existingBooking) {
      throw std::runtime_error("Booking not found");
    }

    // 执行更新操作
    std::unordered_map<oatpp::String, oatpp::Void> updateParams;
    updateParams["status"] = status;
    updateParams["id"] = id;
    auto result = executeQuery(SQL_UPDATE_BOOKING_STATUS, updateParams);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to update booking status: " + result->getErrorMessage());
    }

    Logger::info("Booking status updated successfully for ID: %d, new status: %s", 
      static_cast<int>(id), status->c_str());

    // 返回更新后的预约
    return getBookingById(id);

  } catch (const std::exception& e) {
    Logger::error("Failed to update booking status: %s", e.what());
    throw;
  }
}

bool BookingDao::deleteBooking(const oatpp::Int32& id) {
  try {
    // 检查预约是否存在
    auto existingBooking = getBookingById(id);
    if (!existingBooking) {
      throw std::runtime_error("Booking not found");
    }

    // 开始事务
    auto connection = this->getConnection();
    auto transaction = this->beginTransaction(connection);

    try {
      // 执行删除操作
      std::unordered_map<oatpp::String, oatpp::Void> deleteParams;
      deleteParams["id"] = id;
      auto result = executeQuery(SQL_DELETE_BOOKING, deleteParams, connection);

      if (!result->isSuccess()) {
        throw std::runtime_error("Failed to delete booking: " + result->getErrorMessage());
      }

      // 更新课节的预约人数
      ClassSessionDao sessionDao(this->getExecutor());
      if (!sessionDao.updateBookedCount(existingBooking->session_id, -1, connection)) {
        throw std::runtime_error("Failed to update booked count");
      }

      // 提交事务
      transaction.commit();

      Logger::info("Booking deleted successfully with ID: %d", static_cast<int>(id));
      return true;

    } catch (const std::exception& e) {
      // 回滚事务
      transaction.rollback();
      throw;
    }

  } catch (const std::exception& e) {
    Logger::error("Failed to delete booking: %s", e.what());
    throw;
  }
}
