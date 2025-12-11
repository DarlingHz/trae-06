#include <gtest/gtest.h>
#include <memory>
#include "service/UserService.h"
#include "repository/DatabasePool.h"
#include "repository/UserRepository.h"
#include "auth/JWT.h"
#include <stdexcept>

class UserServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_pool_ = std::make_shared<repository::DatabasePool>(":memory:", 2);
        db_pool_->initialize_tables();
        user_repo_ = repository::create_user_repository(*db_pool_);
        jwt_ = std::make_shared<auth::JWT>("test_secret_key", 3600);
        user_service_ = std::make_shared<service::UserService>(std::move(user_repo_), jwt_);
    }

    void TearDown() override {
        // Cleanup - no need to clear since we're using in-memory database
    }

    std::shared_ptr<repository::DatabasePool> db_pool_;
    std::unique_ptr<repository::UserRepository> user_repo_;
    std::shared_ptr<auth::JWT> jwt_;
    std::shared_ptr<service::UserService> user_service_;
};

TEST_F(UserServiceTest, TestValidateEmail) {
    // Valid emails
    ASSERT_TRUE(user_service_->validate_email("user@example.com"));
    ASSERT_TRUE(user_service_->validate_email("user.name+tag@example.co.uk"));
    ASSERT_TRUE(user_service_->validate_email("user123@example.com"));
    
    // Invalid emails
    ASSERT_FALSE(user_service_->validate_email("user"));
    ASSERT_FALSE(user_service_->validate_email("user@"));
    ASSERT_FALSE(user_service_->validate_email("user@example"));
    ASSERT_FALSE(user_service_->validate_email("user@.com"));
    ASSERT_FALSE(user_service_->validate_email("@example.com"));
}

TEST_F(UserServiceTest, TestValidatePassword) {
    ASSERT_TRUE(user_service_->validate_password("pass123")); // 6 characters
    ASSERT_TRUE(user_service_->validate_password("password")); // 8 characters
    
    ASSERT_FALSE(user_service_->validate_password("pass")); // 4 characters
    ASSERT_FALSE(user_service_->validate_password("")); // empty
}

TEST_F(UserServiceTest, TestRegisterUser) {
    service::UserRegisterRequest request;
    request.email = "test@example.com";
    request.password = "testpass123";
    request.nickname = "Test User";

    auto user = user_service_->register_user(request);
    
    ASSERT_NE(user, nullptr);
    ASSERT_EQ(user->email(), "test@example.com");
    ASSERT_EQ(user->nickname(), "Test User");
    ASSERT_NE(user->password_hash(), "testpass123"); // Password should be hashed
    ASSERT_GT(user->id(), 0);
}

TEST_F(UserServiceTest, TestRegisterDuplicateEmail) {
    service::UserRegisterRequest request1;
    request1.email = "test@example.com";
    request1.password = "testpass123";
    request1.nickname = "Test User";
    user_service_->register_user(request1);

    // Try to register with same email
    service::UserRegisterRequest request2;
    request2.email = "test@example.com";
    request2.password = "differentpass";
    request2.nickname = "Test User 2";
    
    ASSERT_THROW(user_service_->register_user(request2), std::runtime_error);
}

TEST_F(UserServiceTest, TestRegisterInvalidEmail) {
    service::UserRegisterRequest request;
    request.email = "invalid-email";
    request.password = "testpass123";
    request.nickname = "Test User";
    
    ASSERT_THROW(user_service_->register_user(request), std::invalid_argument);
}

TEST_F(UserServiceTest, TestRegisterShortPassword) {
    service::UserRegisterRequest request;
    request.email = "test@example.com";
    request.password = "pass";
    request.nickname = "Test User";
    
    ASSERT_THROW(user_service_->register_user(request), std::invalid_argument);
}

TEST_F(UserServiceTest, TestLoginUser) {
    // First register
    service::UserRegisterRequest register_req;
    register_req.email = "test@example.com";
    register_req.password = "testpass123";
    register_req.nickname = "Test User";
    user_service_->register_user(register_req);

    // Now login
    service::UserLoginRequest login_req;
    login_req.email = "test@example.com";
    login_req.password = "testpass123";
    
    auto response = user_service_->login_user(login_req);
    
    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(response->user->email(), "test@example.com");
    ASSERT_FALSE(response->token.empty());
}

TEST_F(UserServiceTest, TestLoginInvalidPassword) {
    service::UserRegisterRequest register_req;
    register_req.email = "test@example.com";
    register_req.password = "testpass123";
    register_req.nickname = "Test User";
    user_service_->register_user(register_req);

    service::UserLoginRequest login_req;
    login_req.email = "test@example.com";
    login_req.password = "wrongpassword";
    
    auto response = user_service_->login_user(login_req);
    ASSERT_FALSE(response.has_value());
}

TEST_F(UserServiceTest, TestLoginNonExistentUser) {
    service::UserLoginRequest login_req;
    login_req.email = "nonexistent@example.com";
    login_req.password = "password";
    
    auto response = user_service_->login_user(login_req);
    ASSERT_FALSE(response.has_value());
}

TEST_F(UserServiceTest, TestGetUserById) {
    service::UserRegisterRequest register_req;
    register_req.email = "test@example.com";
    register_req.password = "testpass123";
    register_req.nickname = "Test User";
    auto user = user_service_->register_user(register_req);
    
    auto retrieved_user = user_service_->get_user_by_id(user->id());
    ASSERT_NE(retrieved_user, nullptr);
    ASSERT_EQ(retrieved_user->id(), user->id());
    ASSERT_EQ(retrieved_user->email(), user->email());
}

TEST_F(UserServiceTest, TestGetUserByEmail) {
    service::UserRegisterRequest register_req;
    register_req.email = "test@example.com";
    register_req.password = "testpass123";
    register_req.nickname = "Test User";
    auto user = user_service_->register_user(register_req);
    
    auto retrieved_user = user_service_->get_user_by_email(user->email());
    ASSERT_NE(retrieved_user, nullptr);
    ASSERT_EQ(retrieved_user->email(), user->email());
}

TEST_F(UserServiceTest, TestUpdateUser) {
    service::UserRegisterRequest register_req;
    register_req.email = "test@example.com";
    register_req.password = "testpass123";
    register_req.nickname = "Test User";
    auto user = user_service_->register_user(register_req);

    user->set_nickname("Updated Nickname");
    bool success = user_service_->update_user(user);
    ASSERT_TRUE(success);

    auto updated_user = user_service_->get_user_by_id(user->id());
    ASSERT_EQ(updated_user->nickname(), "Updated Nickname");
}

TEST_F(UserServiceTest, TestHashAndVerifyPassword) {
    std::string password = "testpass123";
    
    // Test that password hashing produces different results each time
    std::string hash1 = user_service_->hash_password(password);
    std::string hash2 = user_service_->hash_password(password);
    
    ASSERT_NE(hash1, hash2);
    
    // Test that verification works
    ASSERT_TRUE(user_service_->verify_password(password, hash1));
    ASSERT_TRUE(user_service_->verify_password(password, hash2));
    
    // Test wrong password
    ASSERT_FALSE(user_service_->verify_password("wrongpassword", hash1));
    
    // Test corrupted hash
    ASSERT_FALSE(user_service_->verify_password(password, "corruptedhash"));
}
