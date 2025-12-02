#ifndef ClassTemplateDao_hpp
#define ClassTemplateDao_hpp

#include <oatpp-sqlite/orm.hpp>
#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)

class ClassTemplateDao : public oatpp::orm::DbClient {
public:
  // 构造函数
  ClassTemplateDao(const std::shared_ptr<oatpp::orm::Executor>& executor) 
    : oatpp::orm::DbClient(executor) 
  {}

  // 创建课程模板
  oatpp::Object<ClassTemplateDto> createClassTemplate(const oatpp::Object<ClassTemplateDto>& templateDto);

  // 根据ID查询课程模板
  oatpp::Object<ClassTemplateDto> getClassTemplateById(const oatpp::Int32& id);

  // 查询所有课程模板，支持按教练ID和最低等级筛选
  oatpp::Vector<oatpp::Object<ClassTemplateDto>> getAllClassTemplates(
    const oatpp::Int32& coachId = nullptr,
    const oatpp::String& levelRequired = nullptr
  );

  // 更新课程模板信息
  oatpp::Object<ClassTemplateDto> updateClassTemplate(const oatpp::Object<ClassTemplateDto>& templateDto);

  // 删除课程模板
  bool deleteClassTemplate(const oatpp::Int32& id);

private:
  // SQL查询语句
  static constexpr const char* SQL_CREATE_CLASS_TEMPLATE = R"(
    INSERT INTO class_templates (title, level_required, capacity, duration_minutes, coach_id) 
    VALUES (:title, :level_required, :capacity, :duration_minutes, :coach_id)
    RETURNING id
  )";

  static constexpr const char* SQL_GET_CLASS_TEMPLATE_BY_ID = R"(
    SELECT * FROM class_templates WHERE id = :id
  )";

  static constexpr const char* SQL_GET_ALL_CLASS_TEMPLATES = R"(
    SELECT * FROM class_templates 
    WHERE (:coach_id IS NULL OR coach_id = :coach_id)
      AND (:level_required IS NULL OR level_required = :level_required)
    ORDER BY id
  )";

  static constexpr const char* SQL_UPDATE_CLASS_TEMPLATE = R"(
    UPDATE class_templates SET title = :title, level_required = :level_required, 
      capacity = :capacity, duration_minutes = :duration_minutes, coach_id = :coach_id 
    WHERE id = :id
  )";

  static constexpr const char* SQL_DELETE_CLASS_TEMPLATE = R"(
    DELETE FROM class_templates WHERE id = :id
  )";
};

#include OATPP_CODEGEN_END(DbClient)

#endif /* ClassTemplateDao_hpp */
