#include "MemberService.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>
#include <ctime>
#include <sstream>
#include <iomanip>

MemberService::MemberService(const std::shared_ptr<MemberDao>& memberDao)
  : m_memberDao(memberDao) {
}

oatpp::Object<MemberDto> MemberService::createMember(const oatpp::Object<CreateMemberRequestDto>& requestDto) {
  try {
    // 验证请求参数
    if (!requestDto->name || requestDto->name->empty()) {
      throw std::runtime_error("Name is required");
    }
    if (!requestDto->phone || requestDto->phone->empty()) {
      throw std::runtime_error("Phone is required");
    }

    // 检查手机号是否已存在
    auto existingMember = m_memberDao->getMemberByPhone(requestDto->phone);
    if (existingMember) {
      throw std::runtime_error("Phone number already exists");
    }

    // 创建会员DTO
    auto memberDto = oatpp::Object<MemberDto>::createShared();
    memberDto->name = requestDto->name;
    memberDto->phone = requestDto->phone;
    memberDto->level = requestDto->level ? requestDto->level : "normal";
    
    // 获取当前时间并格式化为字符串
    auto now = std::time(nullptr);
    auto tm = std::localtime(&now);
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    memberDto->created_at = ss.str();

    // 调用DAO层创建会员
    auto createdMember = m_memberDao->createMember(memberDto);
    Logger::info("Member created successfully with ID: %d", static_cast<int>(createdMember->id));

    return createdMember;

  } catch (const std::exception& e) {
    Logger::error("Failed to create member: %s", e.what());
    throw;
  }
}

oatpp::Object<MemberDto> MemberService::getMemberById(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid member ID");
    }

    auto member = m_memberDao->getMemberById(id);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    return member;

  } catch (const std::exception& e) {
    Logger::error("Failed to get member by ID: %s", e.what());
    throw;
  }
}

oatpp::Object<MemberDto> MemberService::getMemberByPhone(const oatpp::String& phone) {
  try {
    if (!phone || phone->empty()) {
      throw std::runtime_error("Phone number is required");
    }

    auto member = m_memberDao->getMemberByPhone(phone);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    return member;

  } catch (const std::exception& e) {
    Logger::error("Failed to get member by phone: %s", e.what());
    throw;
  }
}

oatpp::Vector<oatpp::Object<MemberDto>> MemberService::getAllMembers() {
  try {
    auto members = m_memberDao->getAllMembers();
    Logger::info("Retrieved %d members", members->size());
    return members;

  } catch (const std::exception& e) {
    Logger::error("Failed to get all members: %s", e.what());
    throw;
  }
}

oatpp::Object<MemberDto> MemberService::updateMember(const oatpp::Int32& id, const oatpp::Object<CreateMemberRequestDto>& requestDto) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid member ID");
    }

    // 检查会员是否存在
    auto existingMember = m_memberDao->getMemberById(id);
    if (!existingMember) {
      throw std::runtime_error("Member not found");
    }

    // 如果更新了手机号，检查是否与其他会员冲突
    if (requestDto->phone && requestDto->phone != existingMember->phone) {
      auto memberWithSamePhone = m_memberDao->getMemberByPhone(requestDto->phone);
      if (memberWithSamePhone) {
        throw std::runtime_error("Phone number already exists");
      }
    }

    // 创建更新后的会员DTO
    auto updatedMemberDto = oatpp::Object<MemberDto>::createShared();
    updatedMemberDto->id = id;
    updatedMemberDto->name = requestDto->name ? requestDto->name : existingMember->name;
    updatedMemberDto->phone = requestDto->phone ? requestDto->phone : existingMember->phone;
    updatedMemberDto->level = requestDto->level ? requestDto->level : existingMember->level;
    updatedMemberDto->created_at = existingMember->created_at;

    // 调用DAO层更新会员
    auto member = m_memberDao->updateMember(updatedMemberDto);
    Logger::info("Member updated successfully with ID: %d", static_cast<int>(id));

    return member;

  } catch (const std::exception& e) {
    Logger::error("Failed to update member: %s", e.what());
    throw;
  }
}

bool MemberService::deleteMember(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid member ID");
    }

    bool deleted = m_memberDao->deleteMember(id);
    if (deleted) {
      Logger::info("Member deleted successfully with ID: %d", static_cast<int>(id));
    }

    return deleted;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete member: %s", e.what());
    throw;
  }
}
