#include "TrainingLogService.hpp"
#include "../util/Logger.hpp"
#include <stdexcept>
#include <chrono>
#include <ctime>

TrainingLogService::TrainingLogService(const std::shared_ptr<TrainingLogDao>& trainingLogDao,
                                         const std::shared_ptr<MemberDao>& memberDao)
  : m_trainingLogDao(trainingLogDao), m_memberDao(memberDao) {
}

oatpp::Object<TrainingLogDto> TrainingLogService::createTrainingLog(const oatpp::Object<CreateTrainingLogRequestDto>& requestDto) {
    // 验证请求参数
    if (!requestDto->member_id || !requestDto->session_id || !requestDto->duration_minutes) {
        throw std::invalid_argument("Invalid request parameters");
    }

    // 检查会员是否存在
    auto member = m_memberDao->getMemberById(requestDto->member_id);
    if (!member) {
        throw std::runtime_error("Member not found");
    }

    // 创建训练日志DTO
    auto trainingLog = TrainingLogDto::createShared();
    trainingLog->member_id = requestDto->member_id;
    trainingLog->session_id = requestDto->session_id;
    trainingLog->duration_minutes = requestDto->duration_minutes;
    trainingLog->notes = requestDto->notes;
    
    // 使用当前时间作为创建时间
    auto currentTime = std::chrono::system_clock::now();
    auto currentTimeSeconds = std::chrono::system_clock::to_time_t(currentTime);
    struct tm tmCurrentTime = {};
    gmtime_r(&currentTimeSeconds, &tmCurrentTime);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &tmCurrentTime);
    trainingLog->created_at = buffer;

    // 调用DAO层创建训练日志
    auto createdTrainingLog = m_trainingLogDao->createTrainingLog(trainingLog);

    if (!createdTrainingLog) {
        throw std::runtime_error("Failed to create training log");
    }

    Logger::info("Training log created for member: " + std::to_string(static_cast<int>(requestDto->member_id)));
    return createdTrainingLog;
}

oatpp::List<oatpp::Object<TrainingLogDto>> TrainingLogService::getMemberTrainingLogs(
  const oatpp::Int32& memberId,
  const oatpp::String& from,
  const oatpp::String& to) {

  // 验证会员是否存在
  auto member = m_memberDao->getMemberById(memberId);
  if (!member) {
    throw std::runtime_error("Member not found");
  }

  // 查询训练记录
  auto trainingLogs = m_trainingLogDao->getMemberTrainingLogs(memberId, from, to);
  if (!trainingLogs) {
    throw std::runtime_error("Failed to get training logs");
  }

  Logger::info("Training logs retrieved for member: " + std::to_string(memberId));
  return trainingLogs;
}