#ifndef SCORE_H
#define SCORE_H

#include <string>
#include <chrono>

namespace model {

class Score {
public:
    Score() = default;
    Score(int id, int leaderboard_id, int user_id, long long score,
          const std::string& extra_data, const std::chrono::system_clock::time_point& created_at,
          const std::chrono::system_clock::time_point& updated_at)
        : id_(id), leaderboard_id_(leaderboard_id), user_id_(user_id), score_(score),
          extra_data_(extra_data), created_at_(created_at), updated_at_(updated_at) {}

    // Getters
    int getId() const { return id_; }
    int getLeaderboardId() const { return leaderboard_id_; }
    int getUserId() const { return user_id_; }
    long long getScore() const { return score_; }
    const std::string& getExtraData() const { return extra_data_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }
    const std::chrono::system_clock::time_point& getUpdatedAt() const { return updated_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setLeaderboardId(int leaderboard_id) { leaderboard_id_ = leaderboard_id; }
    void setUserId(int user_id) { user_id_ = user_id; }
    void setScore(long long score) { score_ = score; }
    void setExtraData(const std::string& extra_data) { extra_data_ = extra_data; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }
    void setUpdatedAt(const std::chrono::system_clock::time_point& updated_at) { updated_at_ = updated_at; }

private:
    int id_ = 0;
    int leaderboard_id_ = 0;
    int user_id_ = 0;
    long long score_ = 0;
    std::string extra_data_;
    std::chrono::system_clock::time_point created_at_;
    std::chrono::system_clock::time_point updated_at_;
};

} // namespace model

#endif // SCORE_H