#ifndef ClassTemplateService_hpp
#define ClassTemplateService_hpp

#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"
#include "../data/ClassTemplateDao.hpp"
#include "../data/CoachDao.hpp"

class ClassTemplateService {
public:
  // 构造函数
  ClassTemplateService(const std::shared_ptr<ClassTemplateDao>& classTemplateDao,
                       const std::shared_ptr<CoachDao>& coachDao);

  // 创建课程模板
  oatpp::Object<ClassTemplateDto> createClassTemplate(const oatpp::Object<CreateClassTemplateRequestDto>& requestDto);

  // 根据ID查询课程模板
  oatpp::Object<ClassTemplateDto> getClassTemplateById(const oatpp::Int32& id);

  // 查询所有课程模板，支持按教练ID和最低等级筛选
  oatpp::Vector<oatpp::Object<ClassTemplateDto>> getAllClassTemplates(
    const oatpp::Int32& coachId = nullptr,
    const oatpp::String& levelRequired = nullptr
  );

  // 更新课程模板
  oatpp::Object<ClassTemplateDto> updateClassTemplate(const oatpp::Int32& id, const oatpp::Object<UpdateClassTemplateRequestDto>& requestDto);

  // 删除课程模板
  bool deleteClassTemplate(const oatpp::Int32& id);

private:
  std::shared_ptr<ClassTemplateDao> m_classTemplateDao;
  std::shared_ptr<CoachDao> m_coachDao;
};

#endif /* ClassTemplateService_hpp */
