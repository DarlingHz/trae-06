#include <gtest/gtest.h>
#include "auth/JWT.h"
#include <chrono>
#include <thread>

class JWTTest : public ::testing::Test {
protected:
    void SetUp() override {
        jwt_ = std::make_unique<auth::JWT>("test_secret_key", 10); // 10 seconds expiration
    }

    std::unique_ptr<auth::JWT> jwt_;
};

TEST_F(JWTTest, TestGenerateAndValidateToken) {
    int user_id = 12345;
    std::string token = jwt_->generate_token(user_id);
    
    ASSERT_FALSE(token.empty());
    ASSERT_GT(token.find("."), 0ULL); // Should have at least one dot separator
    
    auto validated_user_id = jwt_->validate_token(token);
    ASSERT_TRUE(validated_user_id.has_value());
    ASSERT_EQ(validated_user_id.value(), user_id);
}

TEST_F(JWTTest, TestValidateInvalidToken) {
    // Test invalid token format (no dots)
    auto result = jwt_->validate_token("invalidtoken");
    ASSERT_FALSE(result.has_value());
    
    // Test invalid token with wrong signature
    std::string valid_token = jwt_->generate_token(12345);
    // Modify the token to break signature
    std::string invalid_token = valid_token;
    if(!invalid_token.empty()) {
        invalid_token[0] = (invalid_token[0] == 'a' ? 'b' : 'a');
    }
    result = jwt_->validate_token(invalid_token);
    ASSERT_FALSE(result.has_value());
}

TEST_F(JWTTest, TestValidateExpiredToken) {
    // Create a JWT with very short expiration
    auth::JWT short_lived_jwt("test_secret_key", 1); // 1 second
    std::string token = short_lived_jwt.generate_token(12345);
    
    // Wait for token to expire
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    auto result = short_lived_jwt.validate_token(token);
    ASSERT_FALSE(result.has_value());
}

TEST_F(JWTTest, TestValidateDifferentSecret) {
    int user_id = 12345;
    std::string token = jwt_->generate_token(user_id);
    
    // Try to validate with different secret
    auth::JWT different_jwt("different_secret", 10);
    auto result = different_jwt.validate_token(token);
    ASSERT_FALSE(result.has_value());
}

TEST_F(JWTTest, TestMultipleTokens) {
    int user_id1 = 100;
    int user_id2 = 200;
    
    std::string token1 = jwt_->generate_token(user_id1);
    std::string token2 = jwt_->generate_token(user_id2);
    
    // Both tokens should be valid for their respective users
    auto result1 = jwt_->validate_token(token1);
    auto result2 = jwt_->validate_token(token2);
    
    ASSERT_TRUE(result1.has_value());
    ASSERT_EQ(result1.value(), user_id1);
    
    ASSERT_TRUE(result2.has_value());
    ASSERT_EQ(result2.value(), user_id2);
}

TEST_F(JWTTest, TestBase64EncodingDecoding) {
    // Test that base64 encoding and decoding works
    std::string test_string = "Hello, World! This is a test string with special chars: !@#$%^&*()";
    
    // We can't test base64_encode/decode directly (private methods), but we can test through JWT functionality
    std::string token = jwt_->generate_token(12345);
    
    // The token should be decodable and contain user_id
    auto decoded_user_id = jwt_->validate_token(token);
    ASSERT_TRUE(decoded_user_id.has_value());
    ASSERT_EQ(decoded_user_id.value(), 12345);
}

TEST_F(JWTTest, TestEmptySecretKey) {
    ASSERT_THROW(auth::JWT("", 3600), std::invalid_argument);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
