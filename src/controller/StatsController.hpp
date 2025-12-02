#ifndef StatsController_hpp
#define StatsController_hpp

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include "../service/StatsService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class StatsController : public oatpp::web::server::api::ApiController {
public:
  // 构造函数
  OATPP_COMPONENT(std::shared_ptr<StatsService>, m_statsService);
  explicit StatsController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper) {}

  // 获取会员统计数据
  ENDPOINT_INFO(getMemberStats) {
    info->summary = "Get member statistics";
    info->pathParams["id"].description = "Member ID";
    info->addResponse<Object<MemberStatsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/stats/member/{id}", getMemberStats, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_statsService->getMemberStats(id));
  }

  // 获取教练统计数据
  ENDPOINT_INFO(getCoachStats) {
    info->summary = "Get coach statistics";
    info->pathParams["id"].description = "Coach ID";
    info->addResponse<Object<CoachStatsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/stats/coach/{id}", getCoachStats, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_statsService->getCoachStats(id));
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* StatsController_hpp */
