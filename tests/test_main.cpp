#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "parking/config.h"
#include "parking/database.h"
#include "parking/dao.h"
#include "parking/services.h"
#include "parking/controllers.h"

// 使用nlohmann json库
#include <nlohmann/json.hpp>

// 全局测试配置
static const std::string TEST_CONFIG_PATH = "tests/test_config.json";
static std::unique_ptr<Config> test_config;
static std::unique_ptr<Database> test_database;

// 测试用户服务
class UserServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试数据访问层实例
        user_dao = std::make_unique<SQLiteUserDAO>(test_database->get_db());
        session_dao = std::make_unique<SQLiteSessionDAO>(test_database->get_db());
        user_service = std::make_unique<UserService>(user_dao.get(), session_dao.get());
    }

    void TearDown() override {
        // 清理测试数据
        user_dao->delete_all();
        session_dao->delete_all();
    }

    std::unique_ptr<SQLiteUserDAO> user_dao;
    std::unique_ptr<SQLiteSessionDAO> session_dao;
    std::unique_ptr<UserService> user_service;
};

// 测试停车位服务
class ParkingSpotServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用户
        User test_user;
        test_user.name = "测试用户";
        test_user.email = "test@example.com";
        test_user.password_hash = "test_hash";
        user_id = user_service->register_user(test_user);

        // 创建测试数据访问层实例
        spot_dao = std::make_unique<SQLiteParkingSpotDAO>(test_database->get_db());
        spot_service = std::make_unique<ParkingSpotService>(spot_dao.get());
    }

    void TearDown() override {
        // 清理测试数据
        spot_dao->delete_all();
        user_dao->delete_all();
    }

    std::unique_ptr<SQLiteUserDAO> user_dao;
    std::unique_ptr<SQLiteParkingSpotDAO> spot_dao;
    std::unique_ptr<ParkingSpotService> spot_service;
    std::string user_id;
};

// 测试预约服务
class ReservationServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用户
        User test_user;
        test_user.name = "测试用户";
        test_user.email = "test@example.com";
        test_user.password_hash = "test_hash";
        renter_id = user_service->register_user(test_user);

        User owner_user;
        owner_user.name = "车主用户";
        owner_user.email = "owner@example.com";
        owner_user.password_hash = "owner_hash";
        owner_id = user_service->register_user(owner_user);

        // 创建测试停车位
        ParkingSpot spot;
        spot.owner_user_id = owner_id;
        spot.title = "测试停车位";
        spot.address = "测试地址";
        spot.price_per_hour = 10.0;
        spot.daily_available_start = 8 * 3600;
        spot.daily_available_end = 22 * 3600;
        spot.latitude = 39.9042;
        spot.longitude = 116.4074;
        spot_id = spot_service->create_spot(spot);

        // 创建测试数据访问层实例
        reservation_dao = std::make_unique<SQLiteReservationDAO>(test_database->get_db());
        reservation_service = std::make_unique<ReservationService>(reservation_dao.get());
    }

    void TearDown() override {
        // 清理测试数据
        reservation_dao->delete_all();
        spot_dao->delete_all();
        user_dao->delete_all();
    }

    std::unique_ptr<SQLiteUserDAO> user_dao;
    std::unique_ptr<SQLiteParkingSpotDAO> spot_dao;
    std::unique_ptr<SQLiteReservationDAO> reservation_dao;
    std::unique_ptr<ReservationService> reservation_service;
    std::unique_ptr<ParkingSpotService> spot_service;
    std::unique_ptr<UserService> user_service;
    std::string renter_id;
    std::string owner_id;
    std::string spot_id;
};

// 用户服务测试用例
TEST_F(UserServiceTest, RegisterUserSuccess) {
    User user;
    user.name = "张三";
    user.email = "zhangsan@example.com";
    user.password_hash = "password123";

    std::string user_id = user_service->register_user(user);
    EXPECT_FALSE(user_id.empty());
}

TEST_F(UserServiceTest, RegisterUserDuplicateEmail) {
    User user;
    user.name = "张三";
    user.email = "zhangsan@example.com";
    user.password_hash = "password123";

    user_service->register_user(user);

    // 再次注册相同邮箱
    EXPECT_THROW({
        user_service->register_user(user);
    }, ServiceError);
}

TEST_F(UserServiceTest, LoginSuccess) {
    User user;
    user.name = "张三";
    user.email = "zhangsan@example.com";
    user.password_hash = "password123";

    std::string user_id = user_service->register_user(user);

    // 登录
    std::string token = user_service->login("zhangsan@example.com", "password123");
    EXPECT_FALSE(token.empty());
}

TEST_F(UserServiceTest, LoginWrongPassword) {
    User user;
    user.name = "张三";
    user.email = "zhangsan@example.com";
    user.password_hash = "password123";

    user_service->register_user(user);

    // 错误密码
    EXPECT_THROW({
        user_service->login("zhangsan@example.com", "wrongpassword");
    }, ServiceError);
}

// 停车位服务测试用例
TEST_F(ParkingSpotServiceTest, CreateSpotSuccess) {
    ParkingSpot spot;
    spot.owner_user_id = user_id;
    spot.title = "小区停车位";
    spot.address = "北京市朝阳区";
    spot.price_per_hour = 10.0;
    spot.daily_available_start = 8 * 3600;
    spot.daily_available_end = 22 * 3600;
    spot.latitude = 39.9042;
    spot.longitude = 116.4074;

    std::string spot_id = spot_service->create_spot(spot);
    EXPECT_FALSE(spot_id.empty());
}

TEST_F(ParkingSpotServiceTest, UpdateSpotSuccess) {
    ParkingSpot spot;
    spot.owner_user_id = user_id;
    spot.title = "小区停车位";
    spot.address = "北京市朝阳区";
    spot.price_per_hour = 10.0;
    spot.daily_available_start = 8 * 3600;
    spot.daily_available_end = 22 * 3600;
    spot.latitude = 39.9042;
    spot.longitude = 116.4074;

    std::string spot_id = spot_service->create_spot(spot);

    // 更新停车位
    spot.id = spot_id;
    spot.title = "更新后的停车位";
    spot.price_per_hour = 15.0;
    
    bool updated = spot_service->update_spot(spot);
    EXPECT_TRUE(updated);

    // 验证更新
    ParkingSpot updated_spot = spot_service->get_spot(spot_id);
    EXPECT_EQ(updated_spot.title, "更新后的停车位");
    EXPECT_EQ(updated_spot.price_per_hour, 15.0);
}

// 预约服务测试用例
TEST_F(ReservationServiceTest, CreateReservationSuccess) {
    Reservation reservation;
    reservation.spot_id = spot_id;
    reservation.renter_user_id = renter_id;
    reservation.owner_user_id = owner_id;
    reservation.vehicle_plate = "京A12345";
    
    // 设置未来的时间段（明天的10:00-11:00）
    time_t now = time(nullptr);
    reservation.start_time = now + 86400 + 36000; // 明天10:00
    reservation.end_time = now + 86400 + 39600;   // 明天11:00

    std::string reservation_id = reservation_service->create_reservation(reservation);
    EXPECT_FALSE(reservation_id.empty());
}

TEST_F(ReservationServiceTest, CreateReservationConflict) {
    Reservation reservation1;
    reservation1.spot_id = spot_id;
    reservation1.renter_user_id = renter_id;
    reservation1.owner_user_id = owner_id;
    reservation1.vehicle_plate = "京A12345";
    
    time_t now = time(nullptr);
    reservation1.start_time = now + 86400 + 36000; // 明天10:00
    reservation1.end_time = now + 86400 + 39600;   // 明天11:00

    std::string reservation_id1 = reservation_service->create_reservation(reservation1);

    // 创建冲突的预约
    Reservation reservation2;
    reservation2.spot_id = spot_id;
    reservation2.renter_user_id = renter_id;
    reservation2.owner_user_id = owner_id;
    reservation2.vehicle_plate = "京A67890";
    reservation2.start_time = now + 86400 + 37800; // 明天10:30
    reservation2.end_time = now + 86400 + 41400;   // 明天11:30

    EXPECT_THROW({
        reservation_service->create_reservation(reservation2);
    }, ServiceError);
}

TEST_F(ReservationServiceTest, CancelReservationSuccess) {
    Reservation reservation;
    reservation.spot_id = spot_id;
    reservation.renter_user_id = renter_id;
    reservation.owner_user_id = owner_id;
    reservation.vehicle_plate = "京A12345";
    
    time_t now = time(nullptr);
    reservation.start_time = now + 86400 + 36000;
    reservation.end_time = now + 86400 + 39600;

    std::string reservation_id = reservation_service->create_reservation(reservation);

    // 取消预约
    bool canceled = reservation_service->cancel_reservation(reservation_id);
    EXPECT_TRUE(canceled);

    // 验证状态
    Reservation canceled_reservation = reservation_service->get_reservation(reservation_id);
    EXPECT_EQ(canceled_reservation.status, "cancelled");
}

// 主函数
int main(int argc, char **argv) {
    // 初始化Google测试
    ::testing::InitGoogleTest(&argc, argv);

    // 加载测试配置
    test_config = Config::load(TEST_CONFIG_PATH);

    // 初始化测试数据库
    test_database = std::make_unique<Database>(test_config->db_path());

    // 执行数据库初始化脚本
    std::ifstream init_sql("sql/init.sql");
    if (init_sql.is_open()) {
        std::string sql_content((std::istreambuf_iterator<char>(init_sql)), 
                               std::istreambuf_iterator<char>());
        test_database->execute_script(sql_content);
        init_sql.close();
    }

    // 运行测试
    int result = RUN_ALL_TESTS();

    return result;
}
