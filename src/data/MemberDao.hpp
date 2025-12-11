#ifndef MemberDao_hpp
#define MemberDao_hpp

#include <oatpp-sqlite/orm.hpp>
#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"

#include OATPP_CODEGEN_BEGIN(DbClient)

class MemberDao : public oatpp::orm::DbClient {
public:
  // 构造函数
  MemberDao(const std::shared_ptr<oatpp::orm::Executor>& executor) 
    : oatpp::orm::DbClient(executor) 
  {}

  // 创建会员
  oatpp::Object<MemberDto> createMember(const oatpp::Object<MemberDto>& memberDto);

  // 根据ID查询会员
  oatpp::Object<MemberDto> getMemberById(const oatpp::Int32& id);

  // 根据手机号查询会员
  oatpp::Object<MemberDto> getMemberByPhone(const oatpp::String& phone);

  // 查询所有会员
  oatpp::Vector<oatpp::Object<MemberDto>> getAllMembers();

  // 更新会员信息
  oatpp::Object<MemberDto> updateMember(const oatpp::Object<MemberDto>& memberDto);

  // 删除会员
  bool deleteMember(const oatpp::Int32& id);

private:
  // SQL查询语句
  static constexpr const char* SQL_CREATE_MEMBER = R"(
    INSERT INTO members (name, phone, level, created_at) 
    VALUES (:name, :phone, :level, :created_at)
  )";

  static constexpr const char* SQL_GET_MEMBER_BY_ID = R"(
    SELECT * FROM members WHERE id = :id
  )";

  static constexpr const char* SQL_GET_MEMBER_BY_PHONE = R"(
    SELECT * FROM members WHERE phone = :phone
  )";

  static constexpr const char* SQL_GET_ALL_MEMBERS = R"(
    SELECT * FROM members ORDER BY created_at DESC
  )";

  static constexpr const char* SQL_UPDATE_MEMBER = R"(
    UPDATE members SET name = :name, phone = :phone, level = :level 
    WHERE id = :id
  )";

  static constexpr const char* SQL_DELETE_MEMBER = R"(
    DELETE FROM members WHERE id = :id
  )";
};

#include OATPP_CODEGEN_END(DbClient)

#endif /* MemberDao_hpp */
