#ifndef CoachService_hpp
#define CoachService_hpp

#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"
#include "../data/CoachDao.hpp"

class CoachService {
public:
  // 构造函数
  CoachService(const std::shared_ptr<CoachDao>& coachDao);

  // 创建教练
  oatpp::Object<CoachDto> createCoach(const oatpp::Object<CreateCoachRequestDto>& requestDto);

  // 根据ID查询教练
  oatpp::Object<CoachDto> getCoachById(const oatpp::Int32& id);

  // 查询所有教练
  oatpp::Vector<oatpp::Object<CoachDto>> getAllCoaches();

  // 更新教练信息
  oatpp::Object<CoachDto> updateCoach(const oatpp::Int32& id, const oatpp::Object<CreateCoachRequestDto>& requestDto);

  // 删除教练
  bool deleteCoach(const oatpp::Int32& id);

private:
  std::shared_ptr<CoachDao> m_coachDao;
};

#endif /* CoachService_hpp */
