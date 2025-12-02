#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <string>
#include <chrono>

namespace model {

enum class ScoreRule {
    HIGHEST, // 取最高分
    CUMULATIVE // 累计分
};

class Leaderboard {
public:
    Leaderboard() = default;
    Leaderboard(int id, int game_id, const std::string& name, const std::string& region,
                ScoreRule score_rule, const std::chrono::system_clock::time_point& created_at)
        : id_(id), game_id_(game_id), name_(name), region_(region), score_rule_(score_rule), created_at_(created_at) {}

    // Getters
    int getId() const { return id_; }
    int getGameId() const { return game_id_; }
    const std::string& getName() const { return name_; }
    const std::string& getRegion() const { return region_; }
    ScoreRule getScoreRule() const { return score_rule_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setGameId(int game_id) { game_id_ = game_id; }
    void setName(const std::string& name) { name_ = name; }
    void setRegion(const std::string& region) { region_ = region; }
    void setScoreRule(ScoreRule score_rule) { score_rule_ = score_rule; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }

    // 辅助方法
    static ScoreRule fromString(const std::string& str) {
        if (str == "highest" || str == "HIGHEST") {
            return ScoreRule::HIGHEST;
        } else if (str == "cumulative" || str == "CUMULATIVE") {
            return ScoreRule::CUMULATIVE;
        } else {
            throw std::invalid_argument("Invalid score rule: " + str);
        }
    }

    static std::string toString(ScoreRule score_rule) {
        switch (score_rule) {
            case ScoreRule::HIGHEST:
                return "highest";
            case ScoreRule::CUMULATIVE:
                return "cumulative";
            default:
                throw std::invalid_argument("Unknown score rule");
        }
    }

private:
    int id_ = 0;
    int game_id_ = 0;
    std::string name_;
    std::string region_;
    ScoreRule score_rule_ = ScoreRule::HIGHEST;
    std::chrono::system_clock::time_point created_at_;
};

} // namespace model

#endif // LEADERBOARD_H