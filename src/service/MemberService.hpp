#ifndef MemberService_hpp
#define MemberService_hpp

#include <oatpp/core/Types.hpp>
#include "../dto/DTOs.hpp"
#include "../data/MemberDao.hpp"

class MemberService {
public:
  // 构造函数
  MemberService(const std::shared_ptr<MemberDao>& memberDao);

  // 创建会员
  oatpp::Object<MemberDto> createMember(const oatpp::Object<CreateMemberRequestDto>& requestDto);

  // 根据ID查询会员
  oatpp::Object<MemberDto> getMemberById(const oatpp::Int32& id);

  // 根据手机号查询会员
  oatpp::Object<MemberDto> getMemberByPhone(const oatpp::String& phone);

  // 查询所有会员
  oatpp::Vector<oatpp::Object<MemberDto>> getAllMembers();

  // 更新会员信息
  oatpp::Object<MemberDto> updateMember(const oatpp::Int32& id, const oatpp::Object<CreateMemberRequestDto>& requestDto);

  // 删除会员
  bool deleteMember(const oatpp::Int32& id);

private:
  std::shared_ptr<MemberDao> m_memberDao;
};

#endif /* MemberService_hpp */
