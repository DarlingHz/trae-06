#ifndef StatsService_hpp
#define StatsService_hpp

#include <memory>
#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"
#include "../data/BookingDao.hpp"
#include "../data/TrainingLogDao.hpp"
#include "../data/ClassSessionDao.hpp"

class StatsService {
public:
  // 构造函数
  StatsService(std::shared_ptr<BookingDao> bookingDao,
               std::shared_ptr<TrainingLogDao> trainingLogDao,
               std::shared_ptr<ClassSessionDao> classSessionDao)
    : m_bookingDao(bookingDao), m_trainingLogDao(trainingLogDao), m_classSessionDao(classSessionDao) {}

  // 获取会员统计数据
  oatpp::Object<MemberStatsDto> getMemberStats(oatpp::Int32 memberId);

  // 获取教练统计数据
  oatpp::Object<CoachStatsDto> getCoachStats(oatpp::Int32 coachId);

private:
  std::shared_ptr<BookingDao> m_bookingDao;
  std::shared_ptr<TrainingLogDao> m_trainingLogDao;
  std::shared_ptr<ClassSessionDao> m_classSessionDao;
};

#endif /* StatsService_hpp */
