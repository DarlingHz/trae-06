#ifndef ClassSessionController_hpp
#define ClassSessionController_hpp

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include "../service/ClassSessionService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class ClassSessionController : public oatpp::web::server::api::ApiController {
public:
  // 构造函数
  OATPP_COMPONENT(std::shared_ptr<ClassSessionService>, m_classSessionService);
  explicit ClassSessionController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper) {}

  // 创建课节
  ENDPOINT_INFO(createClassSession) {
    info->summary = "Create a new class session";
    info->addConsumes<Object<CreateClassSessionRequestDto>>("application/json");
    info->addResponse<Object<ClassSessionDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
  }
  ENDPOINT("POST", "/api/class_sessions", createClassSession, BODY_DTO(Object<CreateClassSessionRequestDto>, sessionDto)) {
    return createDtoResponse(Status::CODE_201, m_classSessionService->createClassSession(sessionDto));
  }

  // 获取所有课节
  ENDPOINT_INFO(getAllClassSessions) {
    info->summary = "Get all class sessions";
    info->queryParams["from"].description = "Filter by start time from (ISO8601)";
    info->queryParams["to"].description = "Filter by start time to (ISO8601)";
    info->queryParams["coach_id"].description = "Filter by coach ID";
    info->queryParams["template_id"].description = "Filter by class template ID";
    info->addResponse<List<Object<ClassSessionDto>>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("GET", "/api/class_sessions", getAllClassSessions, 
    QUERY(String, from), 
    QUERY(String, to), 
    QUERY(Int32, coachId), 
    QUERY(Int32, templateId)) {
    return createDtoResponse(Status::CODE_200, m_classSessionService->getClassSessions(from, to, coachId, templateId));
  }

  // 根据ID获取课节
  ENDPOINT_INFO(getClassSessionById) {
    info->summary = "Get class session by ID";
    info->pathParams["id"].description = "Class session ID";
    info->addResponse<Object<ClassSessionDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/class_sessions/{id}", getClassSessionById, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_classSessionService->getClassSessionById(id));
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* ClassSessionController_hpp */
