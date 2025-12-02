#ifndef ClassTemplateController_hpp
#define ClassTemplateController_hpp

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include "../service/ClassTemplateService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class ClassTemplateController : public oatpp::web::server::api::ApiController {
public:
  // 构造函数
  OATPP_COMPONENT(std::shared_ptr<ClassTemplateService>, m_classTemplateService);
  explicit ClassTemplateController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper) {}

  // 创建课程模板
  ENDPOINT_INFO(createClassTemplate) {
    info->summary = "Create a new class template";
    info->addConsumes<Object<CreateClassTemplateRequestDto>>("application/json");
    info->addResponse<Object<ClassTemplateDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
  }
  ENDPOINT("POST", "/api/class_templates", createClassTemplate, BODY_DTO(Object<CreateClassTemplateRequestDto>, templateDto)) {
    return createDtoResponse(Status::CODE_201, m_classTemplateService->createClassTemplate(templateDto));
  }

  // 获取所有课程模板
  ENDPOINT_INFO(getAllClassTemplates) {
    info->summary = "Get all class templates";
    info->queryParams["coach_id"].description = "Filter by coach ID";
    info->queryParams["level_required"].description = "Filter by minimum level required";
    info->addResponse<List<Object<ClassTemplateDto>>>(Status::CODE_200, "application/json");
  }
  ENDPOINT("GET", "/api/class_templates", getAllClassTemplates, QUERY(Int32, coachId), QUERY(String, levelRequired)) {
    return createDtoResponse(Status::CODE_200, m_classTemplateService->getAllClassTemplates(coachId, levelRequired));
  }

  // 根据ID获取课程模板
  ENDPOINT_INFO(getClassTemplateById) {
    info->summary = "Get class template by ID";
    info->pathParams["id"].description = "Class template ID";
    info->addResponse<Object<ClassTemplateDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/class_templates/{id}", getClassTemplateById, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_classTemplateService->getClassTemplateById(id));
  }

  // 更新课程模板
  ENDPOINT_INFO(updateClassTemplate) {
    info->summary = "Update class template by ID";
    info->pathParams["id"].description = "Class template ID";
    info->addConsumes<Object<UpdateClassTemplateRequestDto>>("application/json");
    info->addResponse<Object<ClassTemplateDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("PUT", "/api/class_templates/{id}", updateClassTemplate, PATH(Int32, id), BODY_DTO(Object<UpdateClassTemplateRequestDto>, templateDto)) {
    return createDtoResponse(Status::CODE_200, m_classTemplateService->updateClassTemplate(id, templateDto));
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* ClassTemplateController_hpp */
