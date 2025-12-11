#ifndef GAME_H
#define GAME_H

#include <string>
#include <chrono>

namespace model {

class Game {
public:
    Game() = default;
    Game(int id, const std::string& game_key, const std::string& name,
         const std::chrono::system_clock::time_point& created_at)
        : id_(id), game_key_(game_key), name_(name), created_at_(created_at) {}

    // Getters
    int getId() const { return id_; }
    const std::string& getGameKey() const { return game_key_; }
    const std::string& getName() const { return name_; }
    const std::chrono::system_clock::time_point& getCreatedAt() const { return created_at_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setGameKey(const std::string& game_key) { game_key_ = game_key; }
    void setName(const std::string& name) { name_ = name; }
    void setCreatedAt(const std::chrono::system_clock::time_point& created_at) { created_at_ = created_at; }

private:
    int id_ = 0;
    std::string game_key_;
    std::string name_;
    std::chrono::system_clock::time_point created_at_;
};

} // namespace model

#endif // GAME_H