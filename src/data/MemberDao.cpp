#include "MemberDao.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>

oatpp::Object<MemberDto> MemberDao::createMember(const oatpp::Object<MemberDto>& memberDto) {
  try {
    // 检查手机号是否已存在
    auto existingMember = getMemberByPhone(memberDto->phone);
    if (existingMember) {
      throw std::runtime_error("Phone number already exists");
    }

    // 执行插入操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["name"] = memberDto->name;
    params["phone"] = memberDto->phone;
    params["level"] = memberDto->level;
    params["created_at"] = memberDto->created_at;
    auto result = executeQuery(SQL_CREATE_MEMBER, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to create member: " + result->getErrorMessage());
    }

    // 获取插入的ID
    auto insertResult = result->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
    if (insertResult && insertResult->size() > 0) {
      auto newId = insertResult->at(0)->id;
      Logger::info("Member created successfully with ID: %d", static_cast<int>(newId));

      // 返回新创建的会员
      return getMemberById(newId);
    } else {
      throw std::runtime_error("Failed to get inserted member ID");
    }

  } catch (const std::exception& e) {
    Logger::error("Failed to create member: %s", e.what());
    throw;
  }
}

oatpp::Object<MemberDto> MemberDao::getMemberById(const oatpp::Int32& id) {
  try {
    // 执行查询操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["id"] = id;
    auto result = executeQuery(SQL_GET_MEMBER_BY_ID, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get member by ID: " + result->getErrorMessage());
    }

    if (result->hasMoreToFetch()) {
      auto members = result->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
      if (members && members->size() > 0) {
        return members->at(0);
      }
    }

    return nullptr;

  } catch (const std::exception& e) {
    Logger::error("Failed to get member by ID: %s", e.what());
    throw;
  }
}

oatpp::Object<MemberDto> MemberDao::getMemberByPhone(const oatpp::String& phone) {
  try {
    // 执行查询操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["phone"] = phone;
    auto result = executeQuery(SQL_GET_MEMBER_BY_PHONE, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get member by phone: " + result->getErrorMessage());
    }

    if (result->hasMoreToFetch()) {
      auto members = result->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
      if (members && members->size() > 0) {
        return members->at(0);
      }
    }

    return nullptr;

  } catch (const std::exception& e) {
    Logger::error("Failed to get member by phone: %s", e.what());
    throw;
  }
}

oatpp::Vector<oatpp::Object<MemberDto>> MemberDao::getAllMembers() {
  try {
    // 执行查询操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    auto result = executeQuery(SQL_GET_ALL_MEMBERS, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get all members: " + result->getErrorMessage());
    }

    auto members = result->fetch<oatpp::Vector<oatpp::Object<MemberDto>>>();
    if (!members) {
      members = oatpp::Vector<oatpp::Object<MemberDto>>::createShared();
    }

    return members;

  } catch (const std::exception& e) {
    Logger::error("Failed to get all members: %s", e.what());
    throw;
  }
}

oatpp::Object<MemberDto> MemberDao::updateMember(const oatpp::Object<MemberDto>& memberDto) {
  try {
    // 检查会员是否存在
    auto existingMember = getMemberById(memberDto->id);
    if (!existingMember) {
      throw std::runtime_error("Member not found");
    }

    // 检查手机号是否已被其他会员使用
    auto memberByPhone = getMemberByPhone(memberDto->phone);
    if (memberByPhone && memberByPhone->id != memberDto->id) {
      throw std::runtime_error("Phone number already exists");
    }

    // 执行更新操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["name"] = memberDto->name;
    params["phone"] = memberDto->phone;
    params["level"] = memberDto->level;
    params["id"] = memberDto->id;
    auto result = executeQuery(SQL_UPDATE_MEMBER, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to update member: " + result->getErrorMessage());
    }

    Logger::info("Member updated successfully with ID: %d", static_cast<int>(memberDto->id));

    // 返回更新后的会员
    return getMemberById(memberDto->id);

  } catch (const std::exception& e) {
    Logger::error("Failed to update member: %s", e.what());
    throw;
  }
}

bool MemberDao::deleteMember(const oatpp::Int32& id) {
  try {
    // 检查会员是否存在
    auto existingMember = getMemberById(id);
    if (!existingMember) {
      throw std::runtime_error("Member not found");
    }

    // 执行删除操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["id"] = id;
    auto result = executeQuery(SQL_DELETE_MEMBER, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to delete member: " + result->getErrorMessage());
    }

    Logger::info("Member deleted successfully with ID: %d", static_cast<int>(id));
    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete member: %s", e.what());
    throw;
  }
}
