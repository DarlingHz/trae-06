#ifndef BookingService_hpp
#define BookingService_hpp

#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"
#include "../data/BookingDao.hpp"
#include "../data/MemberDao.hpp"
#include "../data/ClassSessionDao.hpp"
#include "../data/TrainingLogDao.hpp"
#include "../data/ClassTemplateDao.hpp"

class BookingService {
public:
  // 构造函数
  BookingService(const std::shared_ptr<BookingDao>& bookingDao,
                const std::shared_ptr<MemberDao>& memberDao,
                const std::shared_ptr<ClassSessionDao>& classSessionDao,
                const std::shared_ptr<TrainingLogDao>& trainingLogDao,
                const std::shared_ptr<ClassTemplateDao>& classTemplateDao);

  // 创建预约
  oatpp::Object<BookingDto> createBooking(const oatpp::Object<CreateBookingRequestDto>& requestDto);

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

  // 取消预约
  oatpp::Object<BookingDto> cancelBooking(const oatpp::Int32& id);

  // 签到上课
  oatpp::Object<BookingDto> attendBooking(const oatpp::Int32& id);

private:
  std::shared_ptr<BookingDao> m_bookingDao;
  std::shared_ptr<MemberDao> m_memberDao;
  std::shared_ptr<ClassSessionDao> m_classSessionDao;
  std::shared_ptr<TrainingLogDao> m_trainingLogDao;
  std::shared_ptr<ClassTemplateDao> m_classTemplateDao;

  // 检查是否允许取消预约（课节开始前30分钟内不允许取消）
  bool isCancellationAllowed(const oatpp::Object<ClassSessionDto>& sessionDto);
};

#endif /* BookingService_hpp */
