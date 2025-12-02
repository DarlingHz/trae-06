#include "ClassSessionService.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>

ClassSessionService::ClassSessionService(const std::shared_ptr<ClassSessionDao>& classSessionDao,
                                           const std::shared_ptr<ClassTemplateDao>& classTemplateDao,
                                           const std::shared_ptr<SessionCache>& sessionCache)
  : m_classSessionDao(classSessionDao), m_classTemplateDao(classTemplateDao), m_sessionCache(sessionCache) {
}

oatpp::Object<ClassSessionDto> ClassSessionService::createClassSession(const oatpp::Object<CreateClassSessionRequestDto>& requestDto) {
  try {
    // 验证请求参数
    if (!requestDto->template_id || requestDto->template_id <= 0) {
      throw std::runtime_error("Invalid template ID");
    }
    if (!requestDto->start_time || requestDto->start_time->empty()) {
      throw std::runtime_error("Start time is required");
    }

    // 检查课程模板是否存在
    auto classTemplate = m_classTemplateDao->getClassTemplateById(requestDto->template_id);
    if (!classTemplate) {
      throw std::runtime_error("Class template not found");
    }

    // 创建课节DTO
    auto classSessionDto = oatpp::Object<ClassSessionDto>::createShared();
    classSessionDto->template_id = requestDto->template_id;
    classSessionDto->start_time = requestDto->start_time;
    classSessionDto->status = "scheduled";
    classSessionDto->capacity = classTemplate->capacity;
    classSessionDto->booked_count = 0;

    // 调用DAO层创建课节
    auto createdClassSession = m_classSessionDao->createClassSession(classSessionDto);
    Logger::info("Class session created successfully with ID: %d", static_cast<int>(createdClassSession->id));

    // 清空缓存
    m_sessionCache->clear();

    return createdClassSession;

  } catch (const std::exception& e) {
    Logger::error("Failed to create class session: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassSessionDto> ClassSessionService::getClassSessionById(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid class session ID");
    }

    auto classSession = m_classSessionDao->getClassSessionById(id);
    if (!classSession) {
      throw std::runtime_error("Class session not found");
    }

    return classSession;

  } catch (const std::exception& e) {
    Logger::error("Failed to get class session by ID: %s", e.what());
    throw;
  }
}

oatpp::List<oatpp::Object<ClassSessionDto>> ClassSessionService::getClassSessions(
  const oatpp::String& from,
  const oatpp::String& to,
  const oatpp::Int32& coachId,
  const oatpp::Int32& templateId
) {
  try {
    // 生成缓存键
    std::string fromStr = from ? from->c_str() : "";
    std::string toStr = to ? to->c_str() : "";
    std::string cacheKey = SessionCache::generateKey(fromStr, toStr, coachId, templateId);

    // 尝试从缓存获取结果
    auto cachedResults = m_sessionCache->getResults(cacheKey);
    if (cachedResults) {
      Logger::info("Retrieved %d class sessions from cache", cachedResults->size());
      return cachedResults;
    }

    // 从数据库获取结果
    auto classSessions = m_classSessionDao->getClassSessions(from, to, coachId, templateId);
    Logger::info("Retrieved %d class sessions from database", classSessions->size());

    // 缓存结果
    m_sessionCache->cacheResults(cacheKey, classSessions);

    return classSessions;

  } catch (const std::exception& e) {
    Logger::error("Failed to get class sessions: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassSessionDto> ClassSessionService::updateClassSession(const oatpp::Int32& id, const oatpp::Object<UpdateClassSessionRequestDto>& requestDto) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid class session ID");
    }

    // 检查课节是否存在
    auto existingClassSession = m_classSessionDao->getClassSessionById(id);
    if (!existingClassSession) {
      throw std::runtime_error("Class session not found");
    }

    // 如果更新了模板ID，检查课程模板是否存在
    if (requestDto->template_id && requestDto->template_id != existingClassSession->template_id) {
      auto classTemplate = m_classTemplateDao->getClassTemplateById(requestDto->template_id);
      if (!classTemplate) {
        throw std::runtime_error("Class template not found");
      }
    }

    // 验证更新参数
    if (requestDto->capacity && requestDto->capacity <= 0) {
      throw std::runtime_error("Capacity must be greater than 0");
    }

    // 创建更新后的课节DTO
    auto updatedClassSessionDto = oatpp::Object<ClassSessionDto>::createShared();
    updatedClassSessionDto->id = id;
    updatedClassSessionDto->template_id = requestDto->template_id ? requestDto->template_id : existingClassSession->template_id;
    updatedClassSessionDto->start_time = requestDto->start_time ? requestDto->start_time : existingClassSession->start_time;
    updatedClassSessionDto->status = requestDto->status ? requestDto->status : existingClassSession->status;
    updatedClassSessionDto->capacity = requestDto->capacity ? requestDto->capacity : existingClassSession->capacity;
    updatedClassSessionDto->booked_count = existingClassSession->booked_count;

    // 调用DAO层更新课节
    auto classSession = m_classSessionDao->updateClassSession(updatedClassSessionDto);
    Logger::info("Class session updated successfully with ID: %d", static_cast<int>(id));

    // 清空缓存
    m_sessionCache->clear();

    return classSession;

  } catch (const std::exception& e) {
    Logger::error("Failed to update class session: %s", e.what());
    throw;
  }
}

bool ClassSessionService::deleteClassSession(const oatpp::Int32& id) {
  try {
    if (!id || id <= 0) {
      throw std::runtime_error("Invalid class session ID");
    }

    bool deleted = m_classSessionDao->deleteClassSession(id);
    if (deleted) {
      Logger::info("Class session deleted successfully with ID: %d", static_cast<int>(id));
      // 清空缓存
      m_sessionCache->clear();
    }

    return deleted;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete class session: %s", e.what());
    throw;
  }
}
