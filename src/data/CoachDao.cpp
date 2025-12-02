#include "CoachDao.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>

oatpp::Object<CoachDto> CoachDao::createCoach(const oatpp::Object<CoachDto>& coachDto) {
  try {
    // 执行插入操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["name"] = coachDto->name;
    params["speciality"] = coachDto->speciality;
    auto result = executeQuery(SQL_CREATE_COACH, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to create coach: " + result->getErrorMessage());
    }

    // 获取插入的ID
    auto insertResult = result->fetch<oatpp::Vector<oatpp::Object<CoachDto>>>();
    if (insertResult->size() == 0) {
      throw std::runtime_error("Failed to get inserted coach ID");
    }
    auto newId = insertResult->at(0)->id;
    Logger::info("Coach created successfully with ID: %d", static_cast<int>(newId));

    // 返回新创建的教练
    return getCoachById(newId);

  } catch (const std::exception& e) {
    Logger::error("Failed to create coach: %s", e.what());
    throw;
  }
}

oatpp::Object<CoachDto> CoachDao::getCoachById(const oatpp::Int32& id) {
  try {
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["id"] = id;
    auto result = executeQuery(SQL_GET_COACH_BY_ID, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get coach by ID: " + result->getErrorMessage());
    }

    auto coaches = result->fetch<oatpp::Vector<oatpp::Object<CoachDto>>>();
    if (coaches->size() > 0) {
      return coaches->at(0);
    }

    return nullptr;

  } catch (const std::exception& e) {
    Logger::error("Failed to get coach by ID: %s", e.what());
    throw;
  }
}

oatpp::Vector<oatpp::Object<CoachDto>> CoachDao::getAllCoaches() {
  try {
    std::unordered_map<oatpp::String, oatpp::Void> params;
    auto result = executeQuery(SQL_GET_ALL_COACHES, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get all coaches: " + result->getErrorMessage());
    }

    auto coaches = result->fetch<oatpp::Vector<oatpp::Object<CoachDto>>>();
    return coaches;

  } catch (const std::exception& e) {
    Logger::error("Failed to get all coaches: %s", e.what());
    throw;
  }
}

oatpp::Object<CoachDto> CoachDao::updateCoach(const oatpp::Object<CoachDto>& coachDto) {
  try {
    // 检查教练是否存在
    auto existingCoach = getCoachById(coachDto->id);
    if (!existingCoach) {
      throw std::runtime_error("Coach not found");
    }

    // 执行更新操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["name"] = coachDto->name;
    params["speciality"] = coachDto->speciality;
    params["id"] = coachDto->id;
    auto result = executeQuery(SQL_UPDATE_COACH, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to update coach: " + result->getErrorMessage());
    }

    Logger::info("Coach updated successfully with ID: %d", static_cast<int>(coachDto->id));

    // 返回更新后的教练
    return getCoachById(coachDto->id);

  } catch (const std::exception& e) {
    Logger::error("Failed to update coach: %s", e.what());
    throw;
  }
}

bool CoachDao::deleteCoach(const oatpp::Int32& id) {
  try {
    // 检查教练是否存在
    auto existingCoach = getCoachById(id);
    if (!existingCoach) {
      throw std::runtime_error("Coach not found");
    }

    // 执行删除操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["id"] = id;
    auto result = executeQuery(SQL_DELETE_COACH, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to delete coach: " + result->getErrorMessage());
    }

    Logger::info("Coach deleted successfully with ID: %d", static_cast<int>(id));
    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete coach: %s", e.what());
    throw;
  }
}
