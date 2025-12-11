#ifndef TrainingLogDao_hpp
#define TrainingLogDao_hpp

#include <oatpp-sqlite/orm.hpp>
#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)

class TrainingLogDao : public oatpp::orm::DbClient {
public:
  // 构造函数
  TrainingLogDao(const std::shared_ptr<oatpp::orm::Executor>& executor) 
    : oatpp::orm::DbClient(executor) 
  {}

  // 创建训练记录
  oatpp::Object<TrainingLogDto> createTrainingLog(const oatpp::Object<TrainingLogDto>& logDto);

  // 根据ID查询训练记录
  oatpp::Object<TrainingLogDto> getTrainingLogById(const oatpp::Int32& id);

  // 查询会员的所有训练记录，支持按时间范围筛选
  oatpp::List<oatpp::Object<TrainingLogDto>> getMemberTrainingLogs(
    const oatpp::Int32& memberId,
    const oatpp::String& from = "",
    const oatpp::String& to = ""
  );

  // 更新训练记录
  oatpp::Object<TrainingLogDto> updateTrainingLog(const oatpp::Object<TrainingLogDto>& logDto);

  // 删除训练记录
  bool deleteTrainingLog(const oatpp::Int32& id);

private:
  // SQL查询语句
  static constexpr const char* SQL_CREATE_TRAINING_LOG = R"(
    INSERT INTO training_logs (member_id, session_id, notes, duration_minutes, calories, created_at) 
    VALUES (:member_id, :session_id, :notes, :duration_minutes, :calories, :created_at)
  )";

  static constexpr const char* SQL_GET_TRAINING_LOG_BY_ID = R"(
    SELECT * FROM training_logs WHERE id = :id
  )";

  static constexpr const char* SQL_GET_MEMBER_TRAINING_LOGS = R"(
    SELECT * FROM training_logs 
    WHERE member_id = :member_id
      AND (:from = '' OR created_at >= :from)
      AND (:to = '' OR created_at <= :to)
    ORDER BY created_at DESC
  )";

  static constexpr const char* SQL_UPDATE_TRAINING_LOG = R"(
    UPDATE training_logs SET member_id = :member_id, session_id = :session_id, 
      notes = :notes, duration_minutes = :duration_minutes, calories = :calories 
    WHERE id = :id
  )";

  static constexpr const char* SQL_DELETE_TRAINING_LOG = R"(
    DELETE FROM training_logs WHERE id = :id
  )";
};

#include OATPP_CODEGEN_END(DbClient)

#endif /* TrainingLogDao_hpp */
