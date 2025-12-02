#ifndef BookingDao_hpp
#define BookingDao_hpp

#include <oatpp-sqlite/orm.hpp>
#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)

class BookingDao : public oatpp::orm::DbClient {
public:
  // 构造函数
  BookingDao(const std::shared_ptr<oatpp::orm::Executor>& executor) 
    : oatpp::orm::DbClient(executor), m_executor(executor) 
  {}
  
  /**
   * Get executor.
   * @return - std::shared_ptr<oatpp::orm::Executor>. 
   */
  std::shared_ptr<oatpp::orm::Executor> getExecutor() const {
    return m_executor;
  }

  // 创建预约
  oatpp::Object<BookingDto> createBooking(const oatpp::Object<BookingDto>& bookingDto);

  // 根据ID查询预约
  oatpp::Object<BookingDto> getBookingById(const oatpp::Int32& id);

  // 查询会员的所有预约记录，支持按状态和是否为未来课程筛选
  oatpp::List<oatpp::Object<BookingDto>> getMemberBookings(
    const oatpp::Int32& memberId,
    const oatpp::String& status = "",
    const oatpp::Boolean& upcoming = false
  );

  // 查询课节的所有预约记录
  oatpp::List<oatpp::Object<BookingDto>> getSessionBookings(const oatpp::Int32& sessionId);

  // 检查会员是否已经预约了某一课节
  bool isMemberBooked(const oatpp::Int32& memberId, const oatpp::Int32& sessionId);

  // 更新预约状态
  oatpp::Object<BookingDto> updateBookingStatus(const oatpp::Int32& id, const oatpp::String& status);

  // 删除预约
  bool deleteBooking(const oatpp::Int32& id);

private:
  // 保存executor作为成员变量
  std::shared_ptr<oatpp::orm::Executor> m_executor;
  
  // SQL查询语句
  static constexpr const char* SQL_CREATE_BOOKING = R"(
    INSERT INTO bookings (member_id, session_id, status, created_at) 
    VALUES (:member_id, :session_id, :status, :created_at)
  )";

  static constexpr const char* SQL_GET_BOOKING_BY_ID = R"(
    SELECT * FROM bookings WHERE id = :id
  )";

  static constexpr const char* SQL_GET_MEMBER_BOOKINGS = R"(
    SELECT b.* FROM bookings b
    LEFT JOIN class_sessions cs ON b.session_id = cs.id
    WHERE b.member_id = :member_id
      AND (:status = '' OR b.status = :status)
      AND (:upcoming = false OR (:upcoming = true AND cs.start_time > CURRENT_TIMESTAMP) 
           OR (:upcoming = false AND cs.start_time <= CURRENT_TIMESTAMP))
    ORDER BY cs.start_time DESC
  )";

  static constexpr const char* SQL_GET_SESSION_BOOKINGS = R"(
    SELECT * FROM bookings WHERE session_id = :session_id
  )";

  static constexpr const char* SQL_IS_MEMBER_BOOKED = R"(
    SELECT COUNT(*) FROM bookings WHERE member_id = :member_id AND session_id = :session_id
  )";

  static constexpr const char* SQL_UPDATE_BOOKING_STATUS = R"(
    UPDATE bookings SET status = :status WHERE id = :id
  )";

  static constexpr const char* SQL_DELETE_BOOKING = R"(
    DELETE FROM bookings WHERE id = :id
  )";
};

#include OATPP_CODEGEN_END(DbClient)

#endif /* BookingDao_hpp */
