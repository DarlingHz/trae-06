#ifndef CoachController_hpp
#define CoachController_hpp

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include "../service/CoachService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class CoachController : public oatpp::web::server::api::ApiController {
public:
  // 构造函数
  OATPP_COMPONENT(std::shared_ptr<CoachService>, m_coachService);
  explicit CoachController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper) {}

  // 创建教练
  ENDPOINT_INFO(createCoach) {
    info->summary = "Create a new coach";
    info->addConsumes<Object<CreateCoachRequestDto>>("application/json");
    info->addResponse<Object<CoachDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
  }
  ENDPOINT("POST", "/api/coaches", createCoach, BODY_DTO(Object<CreateCoachRequestDto>, coachDto)) {
    return createDtoResponse(Status::CODE_201, m_coachService->createCoach(coachDto));
  }

  // 获取所有教练
  ENDPOINT_INFO(getAllCoaches) {
    info->summary = "Get all coaches";
    info->addResponse<List<Object<CoachDto>>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("GET", "/api/coaches", getAllCoaches) {
    return createDtoResponse(Status::CODE_200, m_coachService->getAllCoaches());
  }

  // 根据ID获取教练
  ENDPOINT_INFO(getCoachById) {
    info->summary = "Get coach by ID";
    info->pathParams["id"].description = "Coach ID";
    info->addResponse<Object<CoachDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/coaches/{id}", getCoachById, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_coachService->getCoachById(id));
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* CoachController_hpp */
