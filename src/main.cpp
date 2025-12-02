#include <oatpp/network/Server.hpp>
#include <oatpp/network/tcp/server/ConnectionProvider.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/core/macro/component.hpp>
#include <oatpp/core/utils/ConversionUtils.hpp>
#include <oatpp/core/data/stream/BufferStream.hpp>
#include <oatpp/core/async/Executor.hpp>
#include <oatpp/web/server/HttpRouter.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>

#include "controller/MemberController.hpp"
#include "controller/CoachController.hpp"
#include "controller/ClassTemplateController.hpp"
#include "controller/ClassSessionController.hpp"
#include "controller/BookingController.hpp"
#include "controller/TrainingLogController.hpp"
#include "controller/StatsController.hpp"
#include "data/Database.hpp"
#include "util/Config.hpp"
#include "util/Logger.hpp"

#include "service/MemberService.hpp"
#include "service/CoachService.hpp"
#include "service/ClassTemplateService.hpp"
#include "service/ClassSessionService.hpp"
#include "service/BookingService.hpp"
#include "service/StatsService.hpp"
#include "service/TrainingLogService.hpp"

#include "data/MemberDao.hpp"
#include "data/CoachDao.hpp"
#include "data/ClassTemplateDao.hpp"
#include "data/ClassSessionDao.hpp"
#include "data/BookingDao.hpp"
#include "data/TrainingLogDao.hpp"
#include "cache/SessionCache.hpp"

using namespace oatpp;
using namespace oatpp::network;
using namespace oatpp::network::tcp::server;
using namespace oatpp::parser::json::mapping;
using namespace oatpp::data::stream;
using namespace oatpp::web::server;

int main(int argc, const char * argv[]) {
    // 初始化日志（Logger类不需要init方法）
    Logger::info(std::string("Starting gym booking system..."));

    // 读取配置文件
    Config config;
    try {
        config.load("config.json");
        Logger::info(std::string("Configuration loaded successfully"));
    } catch (const std::exception& e) {
        Logger::error(std::string("Failed to load configuration: " + std::string(e.what())));
        return 1;
    }

    // 初始化数据库
    try {
        Database::init(config.getDatabase().path);
        Logger::info(std::string("Database initialized successfully"));
    } catch (const std::exception& e) {
        Logger::error(std::string("Failed to initialize database: " + std::string(e.what())));
        return 1;
    }

    // 创建DAO实例
    auto executor = std::make_shared<oatpp::sqlite::Executor>(Database::getConnectionPool());
    auto memberDao = std::make_shared<MemberDao>(executor);
    auto coachDao = std::make_shared<CoachDao>(executor);
    auto classTemplateDao = std::make_shared<ClassTemplateDao>(executor);
    auto classSessionDao = std::make_shared<ClassSessionDao>(executor);
    auto bookingDao = std::make_shared<BookingDao>(executor);
    auto trainingLogDao = std::make_shared<TrainingLogDao>(executor);

    // 创建缓存实例
    auto sessionCache = std::make_shared<SessionCache>();

    // 创建服务实例
    auto memberService = std::make_shared<MemberService>(memberDao);
    auto coachService = std::make_shared<CoachService>(coachDao);
    auto classTemplateService = std::make_shared<ClassTemplateService>(classTemplateDao, coachDao);
    auto classSessionService = std::make_shared<ClassSessionService>(classSessionDao, classTemplateDao, sessionCache);
    auto bookingService = std::make_shared<BookingService>(bookingDao, memberDao, classSessionDao, trainingLogDao, classTemplateDao);
    auto trainingLogService = std::make_shared<TrainingLogService>(trainingLogDao, memberDao);
    auto statsService = std::make_shared<StatsService>(bookingDao, trainingLogDao, classSessionDao);

    // 创建控制器实例
    auto memberController = std::make_shared<MemberController>();
    auto coachController = std::make_shared<CoachController>();
    auto classTemplateController = std::make_shared<ClassTemplateController>();
    auto classSessionController = std::make_shared<ClassSessionController>();
    auto bookingController = std::make_shared<BookingController>();
    auto trainingLogController = std::make_shared<TrainingLogController>();
    auto statsController = std::make_shared<StatsController>();

    // 设置控制器的服务实例
    memberController->m_memberService = memberService;
    coachController->m_coachService = coachService;
    classTemplateController->m_classTemplateService = classTemplateService;
    classSessionController->m_classSessionService = classSessionService;
    bookingController->m_bookingService = bookingService;
    trainingLogController->m_trainingLogService = trainingLogService;
    statsController->m_statsService = statsService;

    // 创建路由组件
    auto router = oatpp::web::server::HttpRouter::createShared();

    // 注册控制器路由
    router->addController(memberController);
    router->addController(coachController);
    router->addController(classTemplateController);
    router->addController(classSessionController);
    router->addController(bookingController);
    router->addController(trainingLogController);
    router->addController(statsController);

    // 创建连接提供者
    auto address = oatpp::network::Address("0.0.0.0", config.getServer().port, oatpp::network::Address::IP_4);
    auto connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared(address);

    // 创建服务器
    auto connectionHandler = HttpConnectionHandler::createShared(router);
    auto server = Server::createShared(connectionProvider, connectionHandler);

    // 启动服务器
    std::string serverStartMsg = "Server started on port " + oatpp::utils::conversion::int32ToStr(config.getServer().port);
    Logger::info(serverStartMsg);
    std::string maxThreadsMsg = "Maximum threads: " + oatpp::utils::conversion::int32ToStr(config.getServer().threads);
    Logger::info(maxThreadsMsg);

    try {
        server->run();
    } catch (const std::exception& e) {
        std::string errorMsg = "Server failed to run: " + std::string(e.what());
        Logger::error(errorMsg);
        return 1;
    }

    return 0;
}
