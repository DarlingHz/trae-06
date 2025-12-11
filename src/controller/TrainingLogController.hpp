#ifndef TrainingLogController_hpp
#define TrainingLogController_hpp

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include "../service/TrainingLogService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class TrainingLogController : public oatpp::web::server::api::ApiController {
public:
  // 构造函数
  OATPP_COMPONENT(std::shared_ptr<TrainingLogService>, m_trainingLogService);
  explicit TrainingLogController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper) {}

  // 创建自助训练记录
  ENDPOINT_INFO(createTrainingLog) {
    info->summary = "Create a new training log";
    info->addConsumes<Object<CreateTrainingLogRequestDto>>("application/json");
    info->addResponse<Object<TrainingLogDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
  }
  ENDPOINT("POST", "/api/training_logs", createTrainingLog, BODY_DTO(Object<CreateTrainingLogRequestDto>, logDto)) {
    return createDtoResponse(Status::CODE_201, m_trainingLogService->createTrainingLog(logDto));
  }

  // 查看某会员的训练记录
  ENDPOINT_INFO(getMemberTrainingLogs) {
    info->summary = "Get all training logs for a member";
    info->pathParams["id"].description = "Member ID";
    info->queryParams["from"].description = "Filter by start time from (ISO8601)";
    info->queryParams["to"].description = "Filter by start time to (ISO8601)";
    info->addResponse<List<Object<TrainingLogDto>>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/members/{id}/training_logs", getMemberTrainingLogs, 
    PATH(Int32, id), 
    QUERY(String, from), 
    QUERY(String, to)) {
    return createDtoResponse(Status::CODE_200, m_trainingLogService->getMemberTrainingLogs(id, from, to));
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* TrainingLogController_hpp */
