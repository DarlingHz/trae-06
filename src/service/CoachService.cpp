#include "CoachService.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>

CoachService::CoachService(const std::shared_ptr<CoachDao>& coachDao)
  : m_coachDao(coachDao) {
}

oatpp::Object<CoachDto> CoachService::createCoach(const oatpp::Object<CreateCoachRequestDto>& requestDto) {
  try {
    // 验证请求参数
    if (!requestDto->name || requestDto->name->empty()) {
      throw std::runtime_error("Name is required");
    }
    if (!requestDto->speciality || requestDto->speciality->empty()) {
      throw std::runtime_error("Speciality is required");
    }

    // 创建教练DTO
    auto coachDto = oatpp::Object<CoachDto>::createShared();
    coachDto->name = requestDto->name;
    coachDto->speciality = requestDto->speciality;

    // 调用DAO层创建教练
    auto createdCoach = m_coachDao->createCoach(coachDto);
    Logger::info("Coach created successfully with ID: %d", static_cast<int>(createdCoach->id));

    return createdCoach;

  } catch (const std::exception& e) {
    Logger::error("Failed to create coach: %s", e.what());
    throw;
  }
}

oatpp::Object<CoachDto> CoachService::getCoachById(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid coach ID");
    }

    auto coach = m_coachDao->getCoachById(id);
    if (!coach) {
      throw std::runtime_error("Coach not found");
    }

    return coach;

  } catch (const std::exception& e) {
    Logger::error("Failed to get coach by ID: %s", e.what());
    throw;
  }
}

oatpp::Vector<oatpp::Object<CoachDto>> CoachService::getAllCoaches() {
  try {
    auto coaches = m_coachDao->getAllCoaches();
    Logger::info("Retrieved %d coaches", coaches->size());
    return coaches;

  } catch (const std::exception& e) {
    Logger::error("Failed to get all coaches: %s", e.what());
    throw;
  }
}

oatpp::Object<CoachDto> CoachService::updateCoach(const oatpp::Int32& id, const oatpp::Object<CreateCoachRequestDto>& requestDto) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid coach ID");
    }

    // 检查教练是否存在
    auto existingCoach = m_coachDao->getCoachById(id);
    if (!existingCoach) {
      throw std::runtime_error("Coach not found");
    }

    // 创建更新后的教练DTO
    auto updatedCoachDto = oatpp::Object<CoachDto>::createShared();
    updatedCoachDto->id = id;
    updatedCoachDto->name = requestDto->name ? requestDto->name : existingCoach->name;
    updatedCoachDto->speciality = requestDto->speciality ? requestDto->speciality : existingCoach->speciality;

    // 调用DAO层更新教练
    auto coach = m_coachDao->updateCoach(updatedCoachDto);
    Logger::info("Coach updated successfully with ID: %d", static_cast<int>(id));

    return coach;

  } catch (const std::exception& e) {
    Logger::error("Failed to update coach: %s", e.what());
    throw;
  }
}

bool CoachService::deleteCoach(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid coach ID");
    }

    bool deleted = m_coachDao->deleteCoach(id);
    if (deleted) {
      Logger::info("Coach deleted successfully with ID: %d", static_cast<int>(id));
    }

    return deleted;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete coach: %s", e.what());
    throw;
  }
}
