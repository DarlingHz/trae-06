#ifndef SLEEPSESSION_H
#define SLEEPSESSION_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace model {

struct SleepSession {
    int id;
    int user_id;
    std::string start_time;
    std::string end_time;
    int quality;
    std::vector<std::string> tags;
    std::string note;
};

} // namespace model

// 为model::SleepSession结构体添加JSON序列化函数
namespace nlohmann {
    template <>
    struct adl_serializer<model::SleepSession> {
        static void to_json(json& j, const model::SleepSession& session) {
            j = json{
                {"id", session.id},
                {"user_id", session.user_id},
                {"start_time", session.start_time},
                {"end_time", session.end_time},
                {"quality", session.quality},
                {"tags", session.tags},
                {"note", session.note}
            };
        }

        static void from_json(const json& j, model::SleepSession& session) {
            j.at("id").get_to(session.id);
            j.at("user_id").get_to(session.user_id);
            j.at("start_time").get_to(session.start_time);
            j.at("end_time").get_to(session.end_time);
            j.at("quality").get_to(session.quality);
            j.at("tags").get_to(session.tags);
            j.at("note").get_to(session.note);
        }
    };
} // namespace nlohmann

#endif // SLEEPSESSION_H
