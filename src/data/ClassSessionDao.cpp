#include "ClassSessionDao.hpp"
#include "../util/Logger.hpp"
#include "ClassTemplateDao.hpp"
#include "CoachDao.hpp"
#include <stdexcept>

oatpp::Object<ClassSessionDto> ClassSessionDao::createClassSession(const oatpp::Object<ClassSessionDto>& sessionDto) {
  try {
    // 检查课程模板是否存在
    ClassTemplateDao templateDao(this->getExecutor());
    auto templateObj = templateDao.getClassTemplateById(sessionDto->template_id);
    if (!templateObj) {
      throw std::runtime_error("Class template not found");
    }

    // 如果没有指定容量，则使用模板的容量
    auto capacity = sessionDto->capacity;
    if (!capacity) {
      capacity = templateObj->capacity;
    }

    // 创建课节DTO
    auto newSessionDto = oatpp::Object<ClassSessionDto>::createShared();
    newSessionDto->template_id = sessionDto->template_id;
    newSessionDto->start_time = sessionDto->start_time;
    newSessionDto->status = "scheduled";
    newSessionDto->capacity = capacity;
    newSessionDto->booked_count = 0;

    // 执行插入操作
    auto result = executeQuery(SQL_CREATE_CLASS_SESSION,
      std::unordered_map<oatpp::String, oatpp::Void>({
        {"template_id", newSessionDto->template_id},
        {"start_time", newSessionDto->start_time},
        {"status", newSessionDto->status},
        {"capacity", newSessionDto->capacity},
        {"booked_count", newSessionDto->booked_count}
      }));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to create class session: " + result->getErrorMessage());
    }

    // 获取插入的ID
    auto insertResult = executeQuery("SELECT last_insert_rowid() as id", {});
    auto insertRows = insertResult->fetch<oatpp::Vector<oatpp::Fields<oatpp::Any>>>();
    if (!insertRows || insertRows->empty()) {
      throw std::runtime_error("Failed to retrieve inserted class session ID");
    }

    // 提取ID值（使用Any的retrieve方法）
    oatpp::Any idValue = insertRows->at(0)["id"];
    auto idInt64 = idValue.template retrieve<oatpp::Int64>();
    oatpp::Int32 newId = oatpp::Int32(static_cast<v_int32>(idInt64));

    Logger::info("Class session created successfully with ID: %d", static_cast<int>(newId));

    // 返回新创建的课节
    return getClassSessionById(newId);

  } catch (const std::exception& e) {
    Logger::error("Failed to create class session: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassSessionDto> ClassSessionDao::getClassSessionById(const oatpp::Int32& id) {
  try {
    auto result = executeQuery(SQL_GET_CLASS_SESSION_BY_ID, 
      std::unordered_map<oatpp::String, oatpp::Void>({{"id", id}}));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get class session by ID: " + result->getErrorMessage());
    }

    auto sessions = result->fetch<oatpp::Vector<oatpp::Object<ClassSessionDto>>>();
    if (sessions->size() > 0) {
      auto sessionDto = sessions->at(0);

      // 获取课程模板信息
      ClassTemplateDao templateDao(this->getExecutor());
      auto templateDto = templateDao.getClassTemplateById(sessionDto->template_id);
      if (templateDto) {
        sessionDto->template_title = templateDto->title;
        sessionDto->template_duration = templateDto->duration_minutes;
        sessionDto->coach_id = templateDto->coach_id;

        // 获取教练信息
      CoachDao coachDao(this->getExecutor());
      auto coachDto = coachDao.getCoachById(sessionDto->coach_id);
      if (coachDto) {
        sessionDto->coach_name = coachDto->name;
      }

      return sessionDto;
    }
      }

      return nullptr;

    } catch (const std::exception& e) {
      Logger::error("Failed to get class session by ID: %s", e.what());
      throw;
    }
}

oatpp::List<oatpp::Object<ClassSessionDto>> ClassSessionDao::getClassSessions(
  const oatpp::String& from,
  const oatpp::String& to,
  const oatpp::Int32& coachId,
  const oatpp::Int32& templateId
) {
  try {
    auto result = executeQuery(SQL_GET_CLASS_SESSIONS, 
      std::unordered_map<oatpp::String, oatpp::Void>({
        {"from", from},
        {"to", to},
        {"coach_id", coachId},
        {"template_id", templateId}
      }));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get class sessions: " + result->getErrorMessage());
    }

    auto sessions = result->fetch<oatpp::Vector<oatpp::Object<ClassSessionDto>>>();
    auto sessionList = oatpp::List<oatpp::Object<ClassSessionDto>>::createShared();
    for (auto& sessionDto : *sessions) {

      // 获取课程模板信息
      ClassTemplateDao templateDao(this->getExecutor());
      auto templateDto = templateDao.getClassTemplateById(sessionDto->template_id);
      if (templateDto) {
        sessionDto->template_title = templateDto->title;
        sessionDto->template_duration = templateDto->duration_minutes;
        sessionDto->coach_id = templateDto->coach_id;

        // 获取教练信息
      CoachDao coachDao(this->getExecutor());
      auto coachDto = coachDao.getCoachById(sessionDto->coach_id);
      if (coachDto) {
        sessionDto->coach_name = coachDto->name;
      }

      sessionList->push_back(sessionDto);
    }
    }

    return sessionList;

  } catch (const std::exception& e) {
    Logger::error("Failed to get class sessions: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassSessionDto> ClassSessionDao::updateClassSession(const oatpp::Object<ClassSessionDto>& sessionDto) {
  try {
    // 检查课节是否存在
    auto existingSession = getClassSessionById(sessionDto->id);
    if (!existingSession) {
      throw std::runtime_error("Class session not found");
    }

    // 检查课程模板是否存在（如果模板ID发生变化）
    if (sessionDto->template_id != existingSession->template_id) {
      ClassTemplateDao templateDao(this->getExecutor());
      auto templateObj = templateDao.getClassTemplateById(sessionDto->template_id);
      if (!templateObj) {
        throw std::runtime_error("Class template not found");
      }
    }

    // 执行更新操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["template_id"] = sessionDto->template_id;
    params["start_time"] = sessionDto->start_time;
    params["status"] = sessionDto->status;
    params["capacity"] = sessionDto->capacity;
    params["booked_count"] = sessionDto->booked_count;
    params["id"] = sessionDto->id;
    auto result = executeQuery(SQL_UPDATE_CLASS_SESSION, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to update class session: " + result->getErrorMessage());
    }

    Logger::info("Class session updated successfully with ID: %d", static_cast<int>(sessionDto->id));

    // 返回更新后的课节
    return getClassSessionById(sessionDto->id);

  } catch (const std::exception& e) {
    Logger::error("Failed to update class session: %s", e.what());
    throw;
  }
}

bool ClassSessionDao::updateBookedCount(const oatpp::Int32& sessionId, const int delta) {
  try {
    // 检查课节是否存在
    auto existingSession = getClassSessionById(sessionId);
    if (!existingSession) {
      throw std::runtime_error("Class session not found");
    }

    // 执行更新操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["delta"] = oatpp::Int32(delta);
    params["id"] = sessionId;
    auto result = executeQuery(SQL_UPDATE_BOOKED_COUNT, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to update booked count: " + result->getErrorMessage());
    }

    Logger::info("Booked count updated successfully for class session ID: %d, delta: %d", 
      static_cast<int>(sessionId), delta);

    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to update booked count: %s", e.what());
    throw;
  }
}

bool ClassSessionDao::updateBookedCount(const oatpp::Int32& sessionId, const int delta, const oatpp::provider::ResourceHandle<oatpp::orm::Connection>& connection) {
  try {
    // 检查课节是否存在
    auto existingSession = getClassSessionById(sessionId);
    if (!existingSession) {
      throw std::runtime_error("Class session not found");
    }

    // 执行更新操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["delta"] = oatpp::Int32(delta);
    params["id"] = sessionId;
    auto result = executeQuery(SQL_UPDATE_BOOKED_COUNT, params, connection);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to update booked count: " + result->getErrorMessage());
    }

    Logger::info("Booked count updated successfully for class session ID: %d, delta: %d", 
      static_cast<int>(sessionId), delta);

    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to update booked count: %s", e.what());
    throw;
  }
}

bool ClassSessionDao::deleteClassSession(const oatpp::Int32& id) {
  try {
    // 检查课节是否存在
    auto existingSession = getClassSessionById(id);
    if (!existingSession) {
      throw std::runtime_error("Class session not found");
    }

    // 执行删除操作
    auto result = executeQuery(SQL_DELETE_CLASS_SESSION, 
      std::unordered_map<oatpp::String, oatpp::Void>({{"id", id}}));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to delete class session: " + result->getErrorMessage());
    }

    Logger::info("Class session deleted successfully with ID: %d", static_cast<int>(id));
    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete class session: %s", e.what());
    throw;
  }
}
