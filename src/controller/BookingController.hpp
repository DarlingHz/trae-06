#ifndef BookingController_hpp
#define BookingController_hpp

#include <oatpp/web/server/api/ApiController.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/macro/component.hpp>
#include "../service/BookingService.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class BookingController : public oatpp::web::server::api::ApiController {
public:
  // 构造函数
  OATPP_COMPONENT(std::shared_ptr<BookingService>, m_bookingService);
  explicit BookingController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    : oatpp::web::server::api::ApiController(objectMapper) {}

  // 创建预约
  ENDPOINT_INFO(createBooking) {
    info->summary = "Create a new booking";
    info->addConsumes<Object<CreateBookingRequestDto>>("application/json");
    info->addResponse<Object<BookingDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_409, "application/json");
  }
  ENDPOINT("POST", "/api/bookings", createBooking, BODY_DTO(Object<CreateBookingRequestDto>, bookingDto)) {
    return createDtoResponse(Status::CODE_201, m_bookingService->createBooking(bookingDto));
  }

  // 查看某会员的所有预约记录
  ENDPOINT_INFO(getMemberBookings) {
    info->summary = "Get all bookings for a member";
    info->pathParams["id"].description = "Member ID";
    info->queryParams["status"].description = "Filter by booking status";
    info->queryParams["upcoming"].description = "Filter by upcoming bookings (true/false)";
    info->addResponse<List<Object<BookingDto>>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("GET", "/api/members/{id}/bookings", getMemberBookings, 
    PATH(Int32, id), 
    QUERY(String, status), 
    QUERY(Boolean, upcoming)) {
    return createDtoResponse(Status::CODE_200, m_bookingService->getMemberBookings(id, status, upcoming));
  }

  // 取消预约
  ENDPOINT_INFO(cancelBooking) {
    info->summary = "Cancel a booking";
    info->pathParams["id"].description = "Booking ID";
    info->addResponse<Object<BookingDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("POST", "/api/bookings/{id}/cancel", cancelBooking, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_bookingService->cancelBooking(id));
  }

  // 签到上课
  ENDPOINT_INFO(attendBooking) {
    info->summary = "Attend a booking";
    info->pathParams["id"].description = "Booking ID";
    info->addResponse<Object<BookingDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_400, "application/json");
    info->addResponse<Object<ErrorDto>>(Status::CODE_404, "application/json");
  }
  ENDPOINT("POST", "/api/bookings/{id}/attend", attendBooking, PATH(Int32, id)) {
    return createDtoResponse(Status::CODE_200, m_bookingService->attendBooking(id));
  }
};

#include OATPP_CODEGEN_END(ApiController)

#endif /* BookingController_hpp */
