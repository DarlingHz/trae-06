#ifndef ClassSessionDao_hpp
#define ClassSessionDao_hpp

#include <oatpp-sqlite/orm.hpp>
#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)

class ClassSessionDao : public oatpp::orm::DbClient {
public:
  // 构造函数
  ClassSessionDao(const std::shared_ptr<oatpp::orm::Executor>& executor) 
    : oatpp::orm::DbClient(executor), m_executor(executor) 
  {}
  
  // 获取executor
  std::shared_ptr<oatpp::orm::Executor> getExecutor() const {
    return m_executor;
  }
  
  // 创建课节
  oatpp::Object<ClassSessionDto> createClassSession(const oatpp::Object<ClassSessionDto>& sessionDto);

  // 根据ID查询课节
  oatpp::Object<ClassSessionDto> getClassSessionById(const oatpp::Int32& id);

  // 查询课节，支持按时间范围、教练ID、模板ID筛选
  oatpp::List<oatpp::Object<ClassSessionDto>> getClassSessions(
    const oatpp::String& from = nullptr,
    const oatpp::String& to = nullptr,
    const oatpp::Int32& coachId = nullptr,
    const oatpp::Int32& templateId = nullptr
  );

  // 更新课节信息
  oatpp::Object<ClassSessionDto> updateClassSession(const oatpp::Object<ClassSessionDto>& sessionDto);

  // 更新课节预约人数（增加或减少）
  bool updateBookedCount(const oatpp::Int32& sessionId, const int delta);
  
  // 更新课节预约人数（增加或减少）- 事务版本
  bool updateBookedCount(const oatpp::Int32& sessionId, const int delta, const oatpp::provider::ResourceHandle<oatpp::orm::Connection>& connection);

  // 删除课节
  bool deleteClassSession(const oatpp::Int32& id);
  
private:
  // 保存executor作为成员变量
  std::shared_ptr<oatpp::orm::Executor> m_executor;

private:
  // SQL查询语句
  static constexpr const char* SQL_CREATE_CLASS_SESSION = R"(
    INSERT INTO class_sessions (template_id, start_time, status, capacity, booked_count) 
    VALUES (:template_id, :start_time, :status, :capacity, :booked_count)
  )";

  static constexpr const char* SQL_GET_CLASS_SESSION_BY_ID = R"(
    SELECT * FROM class_sessions WHERE id = :id
  )";

  static constexpr const char* SQL_GET_CLASS_SESSIONS = R"(
    SELECT cs.* FROM class_sessions cs
    LEFT JOIN class_templates ct ON cs.template_id = ct.id
    WHERE (:from IS NULL OR cs.start_time >= :from)
      AND (:to IS NULL OR cs.start_time <= :to)
      AND (:coach_id IS NULL OR ct.coach_id = :coach_id)
      AND (:template_id IS NULL OR cs.template_id = :template_id)
    ORDER BY cs.start_time
  )";

  static constexpr const char* SQL_UPDATE_CLASS_SESSION = R"(
    UPDATE class_sessions SET template_id = :template_id, start_time = :start_time, 
      status = :status, capacity = :capacity, booked_count = :booked_count 
    WHERE id = :id
  )";

  static constexpr const char* SQL_UPDATE_BOOKED_COUNT = R"(
    UPDATE class_sessions SET booked_count = booked_count + :delta 
    WHERE id = :id
  )";

  static constexpr const char* SQL_DELETE_CLASS_SESSION = R"(
    DELETE FROM class_sessions WHERE id = :id
  )";
};

#include OATPP_CODEGEN_END(DbClient)

#endif /* ClassSessionDao_hpp */
