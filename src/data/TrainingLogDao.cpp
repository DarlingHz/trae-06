#include "TrainingLogDao.hpp"
#include "../util/Logger.hpp"
#include "MemberDao.hpp"
#include "ClassSessionDao.hpp"
#include <stdexcept>

oatpp::Object<TrainingLogDto> TrainingLogDao::createTrainingLog(const oatpp::Object<TrainingLogDto>& logDto) {
  try {
    // 检查会员是否存在
    MemberDao memberDao(getExecutor());
    auto member = memberDao.getMemberById(logDto->member_id);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    // 如果训练记录关联了课节，检查课节是否存在
    if (logDto->session_id) {
      ClassSessionDao sessionDao(getExecutor());
      auto session = sessionDao.getClassSessionById(logDto->session_id);
      if (!session) {
        throw std::runtime_error("Class session not found");
      }
    }

    // 执行插入操作
    auto result = executeOne(SQL_CREATE_TRAINING_LOG, 
      oatpp::orm::Params({
        {"member_id", logDto->member_id},
        {"session_id", logDto->session_id},
        {"notes", logDto->notes},
        {"duration_minutes", logDto->duration_minutes},
        {"calories", logDto->calories},
        {"created_at", logDto->created_at}
      }));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to create training log: " + result->getErrorMessage());
    }

    // 获取插入的ID
    auto newId = result->getLastInsertRowId();
    Logger::info("Training log created successfully with ID: %d", newId);

    // 返回新创建的训练记录
    return getTrainingLogById(newId);

  } catch (const std::exception& e) {
    Logger::error("Failed to create training log: %s", e.what());
    throw;
  }
}

oatpp::Object<TrainingLogDto> TrainingLogDao::getTrainingLogById(const oatpp::Int32& id) {
  try {
    auto result = executeOne(SQL_GET_TRAINING_LOG_BY_ID, 
      oatpp::orm::Params({{"id", id}}));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get training log by ID: " + result->getErrorMessage());
    }

    if (result->hasMoreToFetch()) {
      auto row = result->fetchRow();
      return oatpp::Object<TrainingLogDto>::createShared(row);
    }

    return nullptr;

  } catch (const std::exception& e) {
    Logger::error("Failed to get training log by ID: %s", e.what());
    throw;
  }
}

oatpp::List<oatpp::Object<TrainingLogDto>> TrainingLogDao::getMemberTrainingLogs(
  const oatpp::Int32& memberId,
  const oatpp::String& from,
  const oatpp::String& to
) {
  try {
    // 检查会员是否存在
    MemberDao memberDao(getExecutor());
    auto member = memberDao.getMemberById(memberId);
    if (!member) {
      throw std::runtime_error("Member not found");
    }

    auto result = executeMany(SQL_GET_MEMBER_TRAINING_LOGS, 
      oatpp::orm::Params({
        {"member_id", memberId},
        {"from", from},
        {"to", to}
      }));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to get member training logs: " + result->getErrorMessage());
    }

    auto logs = oatpp::List<oatpp::Object<TrainingLogDto>>::createShared();
    while (result->hasMoreToFetch()) {
      auto row = result->fetchRow();
      logs->push_back(oatpp::Object<TrainingLogDto>::createShared(row));
    }

    return logs;

  } catch (const std::exception& e) {
    Logger::error("Failed to get member training logs: %s", e.what());
    throw;
  }
}

oatpp::Object<TrainingLogDto> TrainingLogDao::updateTrainingLog(const oatpp::Object<TrainingLogDto>& logDto) {
  try {
    // 检查训练记录是否存在
    auto existingLog = getTrainingLogById(logDto->id);
    if (!existingLog) {
      throw std::runtime_error("Training log not found");
    }

    // 如果训练记录关联了课节，检查课节是否存在
    if (logDto->session_id) {
      ClassSessionDao sessionDao(getExecutor());
      auto session = sessionDao.getClassSessionById(logDto->session_id);
      if (!session) {
        throw std::runtime_error("Class session not found");
      }
    }

    // 执行更新操作
    auto result = executeOne(SQL_UPDATE_TRAINING_LOG, 
      oatpp::orm::Params({
        {"member_id", logDto->member_id},
        {"session_id", logDto->session_id},
        {"notes", logDto->notes},
        {"duration_minutes", logDto->duration_minutes},
        {"calories", logDto->calories},
        {"id", logDto->id}
      }));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to update training log: " + result->getErrorMessage());
    }

    Logger::info("Training log updated successfully with ID: %d", logDto->id->getValue());

    // 返回更新后的训练记录
    return getTrainingLogById(logDto->id);

  } catch (const std::exception& e) {
    Logger::error("Failed to update training log: %s", e.what());
    throw;
  }
}

bool TrainingLogDao::deleteTrainingLog(const oatpp::Int32& id) {
  try {
    // 检查训练记录是否存在
    auto existingLog = getTrainingLogById(id);
    if (!existingLog) {
      throw std::runtime_error("Training log not found");
    }

    // 执行删除操作
    auto result = executeOne(SQL_DELETE_TRAINING_LOG, 
      oatpp::orm::Params({{"id", id}}));

    if (!result->isSuccess()) {
      throw std::runtime_error("Failed to delete training log: " + result->getErrorMessage());
    }

    Logger::info("Training log deleted successfully with ID: %d", id->getValue());
    return true;

  } catch (const std::exception& e) {
    Logger::error("Failed to delete training log: %s", e.what());
    throw;
  }
}
