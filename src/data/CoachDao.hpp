#ifndef CoachDao_hpp
#define CoachDao_hpp

#include <oatpp-sqlite/orm.hpp>
#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)

class CoachDao : public oatpp::orm::DbClient {
public:
  // 构造函数
  CoachDao(const std::shared_ptr<oatpp::orm::Executor>& executor) 
    : oatpp::orm::DbClient(executor) 
  {}

  // 创建教练
  oatpp::Object<CoachDto> createCoach(const oatpp::Object<CoachDto>& coachDto);

  // 根据ID查询教练
  oatpp::Object<CoachDto> getCoachById(const oatpp::Int32& id);

  // 查询所有教练
  oatpp::Vector<oatpp::Object<CoachDto>> getAllCoaches();

  // 更新教练信息
  oatpp::Object<CoachDto> updateCoach(const oatpp::Object<CoachDto>& coachDto);

  // 删除教练
  bool deleteCoach(const oatpp::Int32& id);

private:
  // SQL查询语句
  static constexpr const char* SQL_CREATE_COACH = R"(
    INSERT INTO coaches (name, speciality) 
    VALUES (:name, :speciality)
  )";

  static constexpr const char* SQL_GET_COACH_BY_ID = R"(
    SELECT * FROM coaches WHERE id = :id
  )";

  static constexpr const char* SQL_GET_ALL_COACHES = R"(
    SELECT * FROM coaches ORDER BY id
  )";

  static constexpr const char* SQL_UPDATE_COACH = R"(
    UPDATE coaches SET name = :name, speciality = :speciality 
    WHERE id = :id
  )";

  static constexpr const char* SQL_DELETE_COACH = R"(
    DELETE FROM coaches WHERE id = :id
  )";
};

#include OATPP_CODEGEN_END(DbClient)

#endif /* CoachDao_hpp */
