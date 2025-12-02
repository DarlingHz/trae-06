// leaderboard_test.cpp
// 单元测试文件，测试leaderboard_service的功能

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include "service/LeaderboardService.h"
#include "repository/SQLiteLeaderboardRepository.h"
#include "repository/SQLiteScoreRepository.h"
#include "model/Score.h"
#include <memory>

using namespace service;
using namespace repository;
using namespace model;

// 测试单个用户第一次提交成绩时创建名次
TEST_CASE("Single user first score submission creates rank", "[leaderboard]") {
    // 删除已经存在的数据库文件
    std::remove("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建SQLiteLeaderboardRepository实例
    std::shared_ptr<repository::LeaderboardRepository> leaderboardRepo = std::make_shared<repository::SQLiteLeaderboardRepository>("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建SQLiteScoreRepository实例
    std::shared_ptr<repository::ScoreRepository> scoreRepo = std::make_shared<repository::SQLiteScoreRepository>("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建LeaderboardService实例
    service::LeaderboardService service(leaderboardRepo, scoreRepo);

    // 准备测试数据
    int leaderboardId = 1;
    int gameId = 1;
    std::string leaderboardName = "Test Leaderboard";
    std::string region = "Global";
    model::ScoreRule scoreRule = model::ScoreRule::HIGHEST;
    auto createdAt = std::chrono::system_clock::now();

    // 创建排行榜
    model::Leaderboard leaderboard;
    leaderboard.setGameId(gameId);
    leaderboard.setName(leaderboardName);
    leaderboard.setRegion(region);
    leaderboard.setScoreRule(scoreRule);
    leaderboard.setCreatedAt(createdAt);

    leaderboardRepo->create(leaderboard);

    // 关闭数据库连接
    leaderboardRepo.reset();
    scoreRepo.reset();

    // 打开新的数据库连接
    leaderboardRepo = std::make_shared<repository::SQLiteLeaderboardRepository>("test.db");
    scoreRepo = std::make_shared<repository::SQLiteScoreRepository>("test.db");
    service = service::LeaderboardService(leaderboardRepo, scoreRepo);

    // 查找刚刚创建的排行榜
    auto found_leaderboard = leaderboardRepo->findByGameIdAndName(gameId, leaderboardName);
    REQUIRE(found_leaderboard.has_value());

    // 获取排行榜的id值
    leaderboardId = found_leaderboard->getId();

    // 提交成绩
    int userId = 1;
    int score = 100;
    bool result = service.submitScore(leaderboardId, userId, score);

    // 验证结果
    REQUIRE(result == true);

    // 获取用户排名
    int rank = service.getUserRank(leaderboardId, userId);

    // 验证排名
    REQUIRE(rank == 1);
}

// 测试同一用户多次提交成绩时只保留更好的成绩
TEST_CASE("Same user multiple submissions only keep better score", "[leaderboard]") {
    // 删除已经存在的数据库文件
    std::remove("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建SQLiteLeaderboardRepository实例
    std::shared_ptr<repository::LeaderboardRepository> leaderboardRepo = std::make_shared<repository::SQLiteLeaderboardRepository>("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建SQLiteScoreRepository实例
    std::shared_ptr<repository::ScoreRepository> scoreRepo = std::make_shared<repository::SQLiteScoreRepository>("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建LeaderboardService实例
    service::LeaderboardService service(leaderboardRepo, scoreRepo);

    // 准备测试数据
    int leaderboardId = 1;
    int gameId = 1;
    std::string leaderboardName = "Test Leaderboard";
    std::string region = "Global";
    model::ScoreRule scoreRule = model::ScoreRule::HIGHEST;
    auto createdAt = std::chrono::system_clock::now();

    // 创建排行榜
    model::Leaderboard leaderboard;
    leaderboard.setGameId(gameId);
    leaderboard.setName(leaderboardName);
    leaderboard.setRegion(region);
    leaderboard.setScoreRule(scoreRule);
    leaderboard.setCreatedAt(createdAt);

    leaderboardRepo->create(leaderboard);

    // 查找刚刚创建的排行榜
    auto found_leaderboard = leaderboardRepo->findByGameIdAndName(gameId, leaderboardName);
    REQUIRE(found_leaderboard.has_value());

    // 获取排行榜的id值
    leaderboardId = found_leaderboard->getId();

    // 提交成绩
    int userId = 1;
    int score1 = 100;
    int score2 = 200;

    // 提交第一次成绩
    bool result1 = service.submitScore(leaderboardId, userId, score1);

    // 验证结果
    REQUIRE(result1 == true);

    // 提交第二次成绩
    bool result2 = service.submitScore(leaderboardId, userId, score2);

    // 验证结果
    REQUIRE(result2 == true);

    // 获取用户排名
    int rank = service.getUserRank(leaderboardId, userId);

    // 验证排名
    REQUIRE(rank == 1);

    // 获取用户分数
    int userScore = service.getUserScore(leaderboardId, userId);

    // 验证分数
    REQUIRE(userScore == score2);
}

// 测试多个用户提交成绩时Top N排名是否正确
TEST_CASE("Multiple users submissions Top N ranking is correct", "[leaderboard]") {
    // 删除已经存在的数据库文件
    std::remove("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建SQLiteLeaderboardRepository实例
    std::shared_ptr<repository::LeaderboardRepository> leaderboardRepo = std::make_shared<repository::SQLiteLeaderboardRepository>("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建SQLiteScoreRepository实例
    std::shared_ptr<repository::ScoreRepository> scoreRepo = std::make_shared<repository::SQLiteScoreRepository>("/Users/soma/code/trae/12-01-10/06/build/test.db");
    // 创建LeaderboardService实例
    service::LeaderboardService service(leaderboardRepo, scoreRepo);

    // 准备测试数据
    int leaderboardId = 1;
    int gameId = 1;
    std::string leaderboardName = "Test Leaderboard";
    std::string region = "Global";
    model::ScoreRule scoreRule = model::ScoreRule::HIGHEST;
    auto createdAt = std::chrono::system_clock::now();

    // 创建排行榜
    model::Leaderboard leaderboard;
    leaderboard.setId(leaderboardId);
    leaderboard.setGameId(gameId);
    leaderboard.setName(leaderboardName);
    leaderboard.setRegion(region);
    leaderboard.setScoreRule(scoreRule);
    leaderboard.setCreatedAt(createdAt);

    leaderboardRepo->create(leaderboard);

    // 提交所有用户的成绩
    std::vector<std::pair<int, int>> userScores = {
        {1, 100},
        {2, 200},
        {3, 150},
        {4, 300},
        {5, 250}
    };

    for (auto& userScore : userScores) {
        bool result = service.submitScore(leaderboardId, userScore.first, userScore.second);
        REQUIRE(result == true);
    }

    // 获取Top 3排名
    int limit = 3;
    std::vector<model::Score> topScores = service.getTopScores(leaderboardId, limit);

    // 验证Top 3排名
    REQUIRE(topScores.size() == static_cast<size_t>(limit));
    REQUIRE(topScores[0].getUserId() == 4);
    REQUIRE(topScores[0].getScore() == 300);
    REQUIRE(topScores[1].getUserId() == 5);
    REQUIRE(topScores[1].getScore() == 250);
    REQUIRE(topScores[2].getUserId() == 2);
    REQUIRE(topScores[2].getScore() == 200);
}

int main(int argc, char* argv[]) {
    // 初始化Catch2测试框架
    Catch::Session session;
    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) {
        return returnCode;
    }

    // 运行测试
    int testResult = session.run();

    return testResult;
}
