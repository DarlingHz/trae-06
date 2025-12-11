#ifndef TrainingLogService_hpp
#define TrainingLogService_hpp

#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"
#include "../data/TrainingLogDao.hpp"
#include "../data/MemberDao.hpp"

class TrainingLogService {
public:
  // 构造函数
  TrainingLogService(const std::shared_ptr<TrainingLogDao>& trainingLogDao,
                     const std::shared_ptr<MemberDao>& memberDao);

  // 创建自助训练记录
  oatpp::Object<TrainingLogDto> createTrainingLog(const oatpp::Object<CreateTrainingLogRequestDto>& requestDto);

  // 查看某会员的训练记录
  oatpp::List<oatpp::Object<TrainingLogDto>> getMemberTrainingLogs(
    const oatpp::Int32& memberId,
    const oatpp::String& from = "",
    const oatpp::String& to = ""
  );

private:
  std::shared_ptr<TrainingLogDao> m_trainingLogDao;
  std::shared_ptr<MemberDao> m_memberDao;
};

#endif /* TrainingLogService_hpp */