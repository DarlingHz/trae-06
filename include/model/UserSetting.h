#ifndef USERSETTING_H
#define USERSETTING_H

#include <string>

namespace model {

struct UserSetting {
    int id;
    int user_id;
    double goal_hours_per_day;
    std::string updated_at;
};

} // namespace model

#endif // USERSETTING_H
