#ifndef ClassSessionService_hpp
#define ClassSessionService_hpp

#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"
#include "../data/ClassSessionDao.hpp"
#include "../data/ClassTemplateDao.hpp"
#include "../cache/SessionCache.hpp"

class ClassSessionService {
public:
  // 构造函数
  ClassSessionService(const std::shared_ptr<ClassSessionDao>& classSessionDao,
                      const std::shared_ptr<ClassTemplateDao>& classTemplateDao,
                      const std::shared_ptr<SessionCache>& sessionCache);

  // 创建课节
  oatpp::Object<ClassSessionDto> createClassSession(const oatpp::Object<CreateClassSessionRequestDto>& requestDto);

  // 根据ID查询课节
  oatpp::Object<ClassSessionDto> getClassSessionById(const oatpp::Int32& id);

  // 查询课节，支持按时间范围、教练ID和模板ID筛选
  oatpp::List<oatpp::Object<ClassSessionDto>> getClassSessions(
    const oatpp::String& from = "",
    const oatpp::String& to = "",
    const oatpp::Int32& coachId = 0,
    const oatpp::Int32& templateId = 0
  );

  // 更新课节信息
  oatpp::Object<ClassSessionDto> updateClassSession(const oatpp::Int32& id, const oatpp::Object<UpdateClassSessionRequestDto>& requestDto);

  // 删除课节
  bool deleteClassSession(const oatpp::Int32& id);

private:
  std::shared_ptr<ClassSessionDao> m_classSessionDao;
  std::shared_ptr<ClassTemplateDao> m_classTemplateDao;
  std::shared_ptr<SessionCache> m_sessionCache;
};

#endif /* ClassSessionService_hpp */
