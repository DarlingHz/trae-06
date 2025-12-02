#include "BookingService.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>
#include <chrono>
#include <ctime>

BookingService::BookingService(const std::shared_ptr<BookingDao>& bookingDao,
                               const std::shared_ptr<MemberDao>& memberDao,
                               const std::shared_ptr<ClassSessionDao>& classSessionDao,
                               const std::shared_ptr<TrainingLogDao>& trainingLogDao,
                               const std::shared_ptr<ClassTemplateDao>& classTemplateDao)
  : m_bookingDao(bookingDao), m_memberDao(memberDao), m_classSessionDao(classSessionDao), m_trainingLogDao(trainingLogDao), m_classTemplateDao(classTemplateDao) {
}

oatpp::Object<BookingDto> BookingService::createBooking(const oatpp::Object<CreateBookingRequestDto>& requestDto) {
  try {
    // 验证请求参数
    if (!requestDto->member_id || requestDto->member_id <= 0) {
      throw std::runtime_error("Invalid member ID");
    }
    if (!requestDto->session_id || requestDto->session_id <= 0) {
      throw std::runtime_error("Invalid session ID");
    }

    // 检查会员是否存在
    auto member = m_memberDao->getMemberById(requestDto->member_id);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    // 检查课节是否存在
    auto session = m_classSessionDao->getClassSessionById(requestDto->session_id);
    if (!session) {
      throw std::runtime_error("Class session not found");
    }

    // 检查课节状态是否为scheduled
    if (session->status != "scheduled") {
      throw std::runtime_error("Class session is not available for booking");
    }

    // 检查课节是否已满
    if (session->booked_count >= session->capacity) {
      throw std::runtime_error("Class session is full");
    }

    // 检查会员是否已经预约过该课节
    if (m_bookingDao->isMemberBooked(requestDto->member_id, requestDto->session_id)) {
      throw std::runtime_error("Member has already booked this class session");
    }

    // 创建预约DTO
    auto bookingDto = oatpp::Object<BookingDto>::createShared();
    bookingDto->member_id = requestDto->member_id;
    bookingDto->session_id = requestDto->session_id;
    bookingDto->status = "booked";
    // 使用当前时间作为创建时间（简单实现，假设created_at是字符串类型）
    auto currentTime = std::chrono::system_clock::now();
    auto currentTimeSeconds = std::chrono::system_clock::to_time_t(currentTime);
    struct tm tmCurrentTime = {};
    gmtime_r(&currentTimeSeconds, &tmCurrentTime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tmCurrentTime);
    bookingDto->created_at = buffer;

    // 调用DAO层创建预约（包含事务管理）
    auto createdBooking = m_bookingDao->createBooking(bookingDto);
    Logger::info("Booking created successfully with ID: %d", static_cast<int>(createdBooking->id));

    return createdBooking;

  } catch (const std::exception& e) {
    Logger::error("Failed to create booking: %s", e.what());
    throw;
  }
}

oatpp::Object<BookingDto> BookingService::getBookingById(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid booking ID");
    }

    auto booking = m_bookingDao->getBookingById(id);
    if (!booking) {
      throw std::runtime_error("Booking not found");
    }

    return booking;

  } catch (const std::exception& e) {
    Logger::error("Failed to get booking by ID: %s", e.what());
    throw;
  }
}

oatpp::List<oatpp::Object<BookingDto>> BookingService::getMemberBookings(
  const oatpp::Int32& memberId,
  const oatpp::String& status,
  const oatpp::Boolean& upcoming
) {
  try {
    if (!memberId || memberId <= 0) {
      throw std::runtime_error("Invalid member ID");
    }

    // 检查会员是否存在
    auto member = m_memberDao->getMemberById(memberId);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    auto bookings = m_bookingDao->getMemberBookings(memberId, status, upcoming);
    Logger::info("Retrieved %d bookings for member ID: %d", bookings->size(), static_cast<int>(memberId));
    return bookings;

  } catch (const std::exception& e) {
    Logger::error("Failed to get member bookings: %s", e.what());
    throw;
  }
}

oatpp::List<oatpp::Object<BookingDto>> BookingService::getSessionBookings(const oatpp::Int32& sessionId) {
  try {
    if (!sessionId || sessionId <= 0) {
      throw std::runtime_error("Invalid session ID");
    }

    // 检查课节是否存在
    auto session = m_classSessionDao->getClassSessionById(sessionId);
    if (!session) {
      throw std::runtime_error("Class session not found");
    }

    auto bookings = m_bookingDao->getSessionBookings(sessionId);
    Logger::info("Retrieved %d bookings for session ID: %d", bookings->size(), static_cast<int>(sessionId));
    return bookings;

  } catch (const std::exception& e) {
    Logger::error("Failed to get session bookings: %s", e.what());
    throw;
  }
}

oatpp::Object<BookingDto> BookingService::cancelBooking(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid booking ID");
    }

    // 检查预约是否存在
    auto booking = m_bookingDao->getBookingById(id);
    if (!booking) {
      throw std::runtime_error("Booking not found");
    }

    // 检查预约状态是否为booked
    if (booking->status != "booked") {
      throw std::runtime_error("Booking cannot be cancelled");
    }

    // 检查课节是否存在
    auto session = m_classSessionDao->getClassSessionById(booking->session_id);
    if (!session) {
      throw std::runtime_error("Class session not found");
    }

    // 检查是否允许取消预约
    if (!isCancellationAllowed(session)) {
      throw std::runtime_error("Cancellation is not allowed within 30 minutes before class starts");
    }

    // 调用DAO层取消预约（包含事务管理）
    auto cancelledBooking = m_bookingDao->updateBookingStatus(id, "cancelled");
    Logger::info("Booking cancelled successfully with ID: %d", static_cast<int>(id));

    return cancelledBooking;

  } catch (const std::exception& e) {
    Logger::error("Failed to cancel booking: %s", e.what());
    throw;
  }
}

oatpp::Object<BookingDto> BookingService::attendBooking(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid booking ID");
    }

    // 检查预约是否存在
    auto booking = m_bookingDao->getBookingById(id);
    if (!booking) {
      throw std::runtime_error("Booking not found");
    }

    // 检查预约状态是否为booked
    if (booking->status != "booked") {
      throw std::runtime_error("Booking cannot be attended");
    }

    // 检查课节是否存在
    auto session = m_classSessionDao->getClassSessionById(booking->session_id);
    if (!session) {
      throw std::runtime_error("Class session not found");
    }

    // 检查课节状态是否为scheduled
    if (session->status != "scheduled") {
      throw std::runtime_error("Class session is not available for attendance");
    }

    // 调用DAO层更新预约状态为attended
    auto attendedBooking = m_bookingDao->updateBookingStatus(id, "attended");
    Logger::info("Booking attended successfully with ID: %d", static_cast<int>(id));

    // 获取课程模板信息，用于创建训练记录
    auto classTemplate = m_classTemplateDao->getClassTemplateById(session->template_id);
    if (!classTemplate) {
      throw std::runtime_error("Class template not found");
    }

    // 创建训练记录
    auto trainingLogDto = oatpp::Object<TrainingLogDto>::createShared();
    trainingLogDto->member_id = booking->member_id;
    trainingLogDto->session_id = booking->session_id;
    trainingLogDto->notes = "Attended class: " + classTemplate->title;
    trainingLogDto->duration_minutes = classTemplate->duration_minutes;
    trainingLogDto->calories = nullptr;

    auto createdTrainingLog = m_trainingLogDao->createTrainingLog(trainingLogDto);
    Logger::info("Training log created successfully with ID: %d for booking ID: %d", 
      static_cast<int>(createdTrainingLog->id), static_cast<int>(id));

    return attendedBooking;

  } catch (const std::exception& e) {
    Logger::error("Failed to attend booking: %s", e.what());
    throw;
  }
}

bool BookingService::isCancellationAllowed(const oatpp::Object<ClassSessionDto>& sessionDto) {
  try {
    // 解析课节开始时间（简单实现，假设时间格式为ISO 8601）
    // 实际项目中应该使用更健壮的时间解析库
    std::string startTimeStr = sessionDto->start_time;
    if (startTimeStr.empty()) {
      throw std::runtime_error("Invalid start time format");
    }

    // 获取当前时间
    auto currentTime = std::chrono::system_clock::now();
    auto currentTimeSeconds = std::chrono::system_clock::to_time_t(currentTime);

    // 假设startTimeStr是ISO 8601格式的字符串，例如"2023-12-02T14:30:00Z"
    // 这里简单地将其转换为time_t（实际项目中应该使用更健壮的方法）
    struct tm tmStartTime = {};
    strptime(startTimeStr.c_str(), "%Y-%m-%dT%H:%M:%SZ", &tmStartTime);
    auto startTimeSeconds = mktime(&tmStartTime);

    // 计算时间差（分钟）
    auto diff = startTimeSeconds - currentTimeSeconds;
    auto diffMinutes = diff / 60;

    // 课节开始前30分钟内不允许取消
    return diffMinutes > 30;

  } catch (const std::exception& e) {
    Logger::error("Failed to check cancellation allowed: %s", e.what());
    throw;
  }
}
