#ifndef HEALTH_CONTROLLER_H
#define HEALTH_CONTROLLER_H

#include <cpp-httplib/httplib.h>
#include <repository/DatabaseRepository.h>

namespace event_signup_service::controller {

class HealthController {
private:
    std::shared_ptr<repository::DatabaseRepository> db_repo_;

public:
    explicit HealthController(std::shared_ptr<repository::DatabaseRepository> db_repo)
        : db_repo_(std::move(db_repo)) {
        if (!db_repo_) {
            throw std::invalid_argument("数据库仓库实例不能为null");
        }
    }

    // 健康检查
    void check_health(const httplib::Request& req, httplib::Response& res);
};

} // namespace event_signup_service::controller

#endif // HEALTH_CONTROLLER_H
