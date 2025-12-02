#ifndef MemberController_hpp
#define MemberController_hpp

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include "../service/MemberService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class MemberController : public oatpp::web::server::api::ApiController {
public:
  // 构造函数
  OATPP_COMPONENT(std::shared_ptr<MemberService>, m_memberService);
  explicit MemberController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper) {}

  // 创建会员
  ENDPOINT_INFO(createMember) {
    info->summary = "Create a new member";
    info->addConsumes<Object<CreateMemberRequestDto>>("application/json");
    info->addResponse<Object<MemberDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_409, "application/json");
  }
  ENDPOINT("POST", "/api/members", createMember, BODY_DTO(Object<CreateMemberRequestDto>, memberDto)) {
    return createDtoResponse(Status::CODE_201, m_memberService->createMember(memberDto));
  }

  // 根据ID获取会员
  ENDPOINT_INFO(getMemberById) {
    info->summary = "Get member by ID";
    info->pathParams["id"].description = "Member ID";
    info->addResponse<Object<MemberDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/members/{id}", getMemberById, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_memberService->getMemberById(id));
  }

  // 根据手机号获取会员
  ENDPOINT_INFO(getMemberByPhone) {
    info->summary = "Get member by phone number";
    info->queryParams["phone"].description = "Phone number";
    info->addResponse<Object<MemberDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/members/by-phone", getMemberByPhone, QUERY(String, phone)) {
    return createDtoResponse(Status::CODE_200, m_memberService->getMemberByPhone(phone));
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* MemberController_hpp */
