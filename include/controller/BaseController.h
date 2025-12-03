#ifndef BASE_CONTROLLER_H
#define BASE_CONTROLLER_H

#include <memory>
#include <service/EventService.h>
#include <service/UserService.h>
#include <service/RegistrationService.h>

namespace event_signup_service::controller {

class BaseController {
protected:
    std::shared_ptr<service::EventService> event_service_;
    std::shared_ptr<service::UserService> user_service_;
    std::shared_ptr<service::RegistrationService> registration_service_;

public:
    BaseController(
        std::shared_ptr<service::EventService> event_service,
        std::shared_ptr<service::UserService> user_service,
        std::shared_ptr<service::RegistrationService> registration_service
    ) : event_service_(std::move(event_service)),
        user_service_(std::move(user_service)),
        registration_service_(std::move(registration_service)) {
        if (!event_service_ || !user_service_ || !registration_service_) {
            throw std::invalid_argument("服务实例不能为null");
        }
    }

    virtual ~BaseController() = default;
};

} // namespace event_signup_service::controller

#endif // BASE_CONTROLLER_H
