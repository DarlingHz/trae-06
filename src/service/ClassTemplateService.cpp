#include "ClassTemplateService.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>

ClassTemplateService::ClassTemplateService(const std::shared_ptr<ClassTemplateDao>& classTemplateDao,
                                             const std::shared_ptr<CoachDao>& coachDao)
  : m_classTemplateDao(classTemplateDao), m_coachDao(coachDao) {
}

oatpp::Object<ClassTemplateDto> ClassTemplateService::createClassTemplate(const oatpp::Object<CreateClassTemplateRequestDto>& requestDto) {
  try {
    // 验证请求参数
    if (!requestDto->title || requestDto->title->empty()) {
      throw std::runtime_error("Title is required");
    }
    if (!requestDto->coach_id || requestDto->coach_id <= 0) {
      throw std::runtime_error("Invalid coach ID");
    }
    if (requestDto->capacity && requestDto->capacity <= 0) {
      throw std::runtime_error("Capacity must be greater than 0");
    }
    if (requestDto->duration_minutes && requestDto->duration_minutes <= 0) {
      throw std::runtime_error("Duration must be greater than 0");
    }

    // 检查教练是否存在
    auto coach = m_coachDao->getCoachById(requestDto->coach_id);
    if (!coach) {
      throw std::runtime_error("Coach not found");
    }

    // 创建课程模板DTO
    auto classTemplateDto = oatpp::Object<ClassTemplateDto>::createShared();
    classTemplateDto->title = requestDto->title;
    classTemplateDto->level_required = requestDto->level_required ? requestDto->level_required : "normal";
    classTemplateDto->capacity = requestDto->capacity ? requestDto->capacity : oatpp::Int32(20);
    classTemplateDto->duration_minutes = requestDto->duration_minutes ? requestDto->duration_minutes : oatpp::Int32(60);
    classTemplateDto->coach_id = requestDto->coach_id;

    // 调用DAO层创建课程模板
    auto createdClassTemplate = m_classTemplateDao->createClassTemplate(classTemplateDto);
    Logger::info("Class template created successfully with ID: %d", static_cast<int>(createdClassTemplate->id));

    return createdClassTemplate;

  } catch (const std::exception& e) {
    Logger::error("Failed to create class template: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassTemplateDto> ClassTemplateService::getClassTemplateById(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid class template ID");
    }

    auto classTemplate = m_classTemplateDao->getClassTemplateById(id);
    if (!classTemplate) {
      throw std::runtime_error("Class template not found");
    }

    return classTemplate;

  } catch (const std::exception& e) {
    Logger::error("Failed to get class template by ID: %s", e.what());
    throw;
  }
}

oatpp::Vector<oatpp::Object<ClassTemplateDto>> ClassTemplateService::getAllClassTemplates(
  const oatpp::Int32& coachId,
  const oatpp::String& levelRequired
) {
  try {
    // 验证请求参数
    if (coachId && coachId <= 0) {
      throw std::runtime_error("Invalid coach ID");
    }
    if (levelRequired && levelRequired->empty()) {
      throw std::runtime_error("Level required cannot be empty");
    }

    auto classTemplates = m_classTemplateDao->getAllClassTemplates(coachId, levelRequired);
    Logger::info("Retrieved %d class templates", classTemplates->size());
    return classTemplates;

  } catch (const std::exception& e) {
    Logger::error("Failed to get all class templates: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassTemplateDto> ClassTemplateService::updateClassTemplate(const oatpp::Int32& id, const oatpp::Object<UpdateClassTemplateRequestDto>& requestDto) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid class template ID");
    }

    // 检查课程模板是否存在
    auto existingClassTemplate = m_classTemplateDao->getClassTemplateById(id);
    if (!existingClassTemplate) {
      throw std::runtime_error("Class template not found");
    }

    // 如果更新了教练ID，检查教练是否存在
    if (requestDto->coach_id && requestDto->coach_id != existingClassTemplate->coach_id) {
      auto coach = m_coachDao->getCoachById(requestDto->coach_id);
      if (!coach) {
        throw std::runtime_error("Coach not found");
      }
    }

    // 验证更新参数
    if (requestDto->capacity && requestDto->capacity <= 0) {
      throw std::runtime_error("Capacity must be greater than 0");
    }
    if (requestDto->duration_minutes && requestDto->duration_minutes <= 0) {
      throw std::runtime_error("Duration must be greater than 0");
    }

    // 创建更新后的课程模板DTO
    auto updatedClassTemplateDto = oatpp::Object<ClassTemplateDto>::createShared();
    updatedClassTemplateDto->id = id;
    updatedClassTemplateDto->title = requestDto->title ? requestDto->title : existingClassTemplate->title;
    updatedClassTemplateDto->level_required = requestDto->level_required ? requestDto->level_required : existingClassTemplate->level_required;
    updatedClassTemplateDto->capacity = requestDto->capacity ? requestDto->capacity : existingClassTemplate->capacity;
    updatedClassTemplateDto->duration_minutes = requestDto->duration_minutes ? requestDto->duration_minutes : existingClassTemplate->duration_minutes;
    updatedClassTemplateDto->coach_id = requestDto->coach_id ? requestDto->coach_id : existingClassTemplate->coach_id;

    // 调用DAO层更新课程模板
    auto classTemplate = m_classTemplateDao->updateClassTemplate(updatedClassTemplateDto);
    Logger::info("Class template updated successfully with ID: %d", static_cast<int>(id));

    return classTemplate;

  } catch (const std::exception& e) {
    Logger::error("Failed to update class template: %s", e.what());
    throw;
  }
}

bool ClassTemplateService::deleteClassTemplate(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid class template ID");
    }

    bool deleted = m_classTemplateDao->deleteClassTemplate(id);
    if (deleted) {
      Logger::info("Class template deleted successfully with ID: %d", static_cast<int>(id));
    }

    return deleted;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete class template: %s", e.what());
    throw;
  }
}
