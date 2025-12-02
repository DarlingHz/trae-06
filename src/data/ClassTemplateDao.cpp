#include "ClassTemplateDao.hpp"
#include "../util/Logger.hpp"
#include "CoachDao.hpp"
#include <stdexcept>

oatpp::Object<ClassTemplateDto> ClassTemplateDao::createClassTemplate(const oatpp::Object<ClassTemplateDto>& templateDto) {
  try {
    // 检查教练是否存在
    std::unordered_map<oatpp::String, oatpp::Void> coachParams;
    coachParams["id"] = templateDto->coach_id;
    auto coachResult = executeQuery("SELECT * FROM coaches WHERE id = :id", coachParams);
    if (!coachResult->isSuccess() || coachResult->fetch<oatpp::Vector<oatpp::Object<CoachDto>>>()->size() == 0) {
      throw std::runtime_error("Coach not found");
    }

    // 执行插入操作
    std::unordered_map<oatpp::String, oatpp::Void> insertParams;
    insertParams["title"] = templateDto->title;
    insertParams["level_required"] = templateDto->level_required;
    insertParams["capacity"] = templateDto->capacity;
    insertParams["duration_minutes"] = templateDto->duration_minutes;
    insertParams["coach_id"] = templateDto->coach_id;
    auto insertResult = executeQuery(SQL_CREATE_CLASS_TEMPLATE, insertParams);

    if (!insertResult->isSuccess()) {
      throw std::runtime_error("Failed to create class template: " + insertResult->getErrorMessage());
    }

    // 获取插入的ID
    auto idVector = insertResult->fetch<oatpp::Vector<oatpp::Fields<oatpp::Any>>>();
    if (!idVector || idVector->empty()) {
      throw std::runtime_error("Failed to retrieve inserted class template ID");
    }

    // 提取ID值（使用Any的retrieve方法）
    oatpp::Any idValue = idVector->at(0)["id"];
    auto idInt64 = idValue.template retrieve<oatpp::Int64>();
    oatpp::Int32 newId = oatpp::Int32(static_cast<v_int32>(idInt64));

    Logger::info("Class template created successfully with ID: %d", static_cast<int>(newId));

    // 返回新创建的课程模板
    return getClassTemplateById(newId);

  } catch (const std::exception& e) {
    Logger::error("Failed to create class template: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassTemplateDto> ClassTemplateDao::updateClassTemplate(const oatpp::Object<ClassTemplateDto>& templateDto) {
  try {
    // 检查课程模板是否存在
    auto existingTemplate = getClassTemplateById(templateDto->id);
    if (!existingTemplate) {
      throw std::runtime_error("Class template not found");
    }

    // 检查教练是否存在
    if (templateDto->coach_id != existingTemplate->coach_id) {
      std::unordered_map<oatpp::String, oatpp::Void> coachParams;
      coachParams["id"] = templateDto->coach_id;
      auto coachResult = executeQuery("SELECT * FROM coaches WHERE id = :id", coachParams);
      if (!coachResult->isSuccess() || coachResult->fetch<oatpp::Vector<oatpp::Object<CoachDto>>>()->size() == 0) {
        throw std::runtime_error("Coach not found");
      }
    }

    // 执行更新操作
    std::unordered_map<oatpp::String, oatpp::Void> updateParams;
    updateParams["title"] = templateDto->title;
    updateParams["level_required"] = templateDto->level_required;
    updateParams["capacity"] = templateDto->capacity;
    updateParams["duration_minutes"] = templateDto->duration_minutes;
    updateParams["coach_id"] = templateDto->coach_id;
    updateParams["id"] = templateDto->id;
    auto updateResult = executeQuery(SQL_UPDATE_CLASS_TEMPLATE, updateParams);

    if (!updateResult->isSuccess()) {
      throw std::runtime_error("Failed to update class template: " + updateResult->getErrorMessage());
    }

    Logger::info("Class template updated successfully with ID: %d", static_cast<int>(templateDto->id));

    // 返回更新后的课程模板
    return getClassTemplateById(templateDto->id);

  } catch (const std::exception& e) {
    Logger::error("Failed to update class template: %s", e.what());
    throw;
  }
}

oatpp::Object<ClassTemplateDto> ClassTemplateDao::getClassTemplateById(const oatpp::Int32& id) {
  try {
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["id"] = id;
    auto result = executeQuery(SQL_GET_CLASS_TEMPLATE_BY_ID, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get class template by ID: " + result->getErrorMessage());
    }

    auto templates = result->fetch<oatpp::Vector<oatpp::Object<ClassTemplateDto>>>();
    if (templates->size() > 0) {
      return templates->at(0);
    }

    return nullptr;

  } catch (const std::exception& e) {
    Logger::error("Failed to get class template by ID: %s", e.what());
    throw;
  }
}

oatpp::Vector<oatpp::Object<ClassTemplateDto>> ClassTemplateDao::getAllClassTemplates(
  const oatpp::Int32& coachId,
  const oatpp::String& levelRequired
) {
  try {
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["coach_id"] = coachId;
    params["level_required"] = levelRequired;
    auto result = executeQuery(SQL_GET_ALL_CLASS_TEMPLATES, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get all class templates: " + result->getErrorMessage());
    }

    auto templates = result->fetch<oatpp::Vector<oatpp::Object<ClassTemplateDto>>>();
    return templates;

  } catch (const std::exception& e) {
    Logger::error("Failed to get all class templates: %s", e.what());
    throw;
  }
}



bool ClassTemplateDao::deleteClassTemplate(const oatpp::Int32& id) {
  try {
    // 检查课程模板是否存在
    auto existingTemplate = getClassTemplateById(id);
    if (!existingTemplate) {
      throw std::runtime_error("Class template not found");
    }

    // 执行删除操作
    std::unordered_map<oatpp::String, oatpp::Void> params;
    params["id"] = id;
    auto result = executeQuery(SQL_DELETE_CLASS_TEMPLATE, params);

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to delete class template: " + result->getErrorMessage());
    }

    Logger::info("Class template deleted successfully with ID: %d", static_cast<int>(id));
    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete class template: %s", e.what());
    throw;
  }
}
