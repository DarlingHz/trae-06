#include <iostream>
#include <memory>
#include "../src/services/user_service.h"
#include "../src/repository/user_repository.h"
#include "../src/cache/cache_manager.h"
#include "test_utils.h"

using namespace std;

void test_user_creation() {
    cout << "\n=== Testing user creation ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto user_service = make_shared<UserService>(user_repo, cache_manager);
    
    User user;
    user.set_username("testuser");
    user.set_password("password123");
    user.set_email("test@example.com");
    user.set_phone("13800138000");
    user.set_real_name("Test User");
    user.set_department("IT");
    user.set_position("Engineer");
    user.set_role(UserRole::ROLE_USER);
    
    user_service->create_user(user);
    
    auto created_user = user_service->get_user_by_id(user.get_id());
    ASSERT_TRUE(created_user.has_value());
    ASSERT_EQUAL(created_user->get_username(), "testuser");
    ASSERT_EQUAL(created_user->get_email(), "test@example.com");
    ASSERT_EQUAL(created_user->get_role(), UserRole::ROLE_USER);
    
    cout << "✓ User creation test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_user_authentication() {
    cout << "\n=== Testing user authentication ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto user_service = make_shared<UserService>(user_repo, cache_manager);
    
    User user;
    user.set_username("authuser");
    user.set_password("password123");
    user.set_email("auth@example.com");
    user_service->create_user(user);
    
    // 测试正确密码
    auto authenticated_user = user_service->authenticate("authuser", "password123");
    ASSERT_TRUE(authenticated_user.has_value());
    
    // 测试错误密码
    auto failed_user = user_service->authenticate("authuser", "wrongpassword");
    ASSERT_FALSE(failed_user.has_value());
    
    // 测试不存在的用户
    auto non_existent_user = user_service->authenticate("nonexistent", "password");
    ASSERT_FALSE(non_existent_user.has_value());
    
    cout << "✓ User authentication test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_user_update() {
    cout << "\n=== Testing user update ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto user_service = make_shared<UserService>(user_repo, cache_manager);
    
    User user;
    user.set_username("updateuser");
    user.set_password("password123");
    user.set_email("update@example.com");
    user_service->create_user(user);
    
    user.set_email("updated@example.com");
    user.set_phone("13900139000");
    user.set_real_name("Updated User");
    user_service->update_user(user);
    
    auto updated_user = user_service->get_user_by_id(user.get_id());
    ASSERT_TRUE(updated_user.has_value());
    ASSERT_EQUAL(updated_user->get_email(), "updated@example.com");
    ASSERT_EQUAL(updated_user->get_phone(), "13900139000");
    ASSERT_EQUAL(updated_user->get_real_name(), "Updated User");
    
    cout << "✓ User update test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_user_deletion() {
    cout << "\n=== Testing user deletion ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto user_service = make_shared<UserService>(user_repo, cache_manager);
    
    User user;
    user.set_username("deleteuser");
    user.set_password("password123");
    user.set_email("delete@example.com");
    user_service->create_user(user);
    
    user_service->delete_user(user.get_id());
    
    auto deleted_user = user_service->get_user_by_id(user.get_id());
    ASSERT_FALSE(deleted_user.has_value());
    
    cout << "✓ User deletion test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_user_list() {
    cout << "\n=== Testing user list ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto user_service = make_shared<UserService>(user_repo, cache_manager);
    
    // 创建多个用户
    for (int i = 0; i < 5; ++i) {
        User user;
        user.set_username("user" + to_string(i));
        user.set_password("password" + to_string(i));
        user.set_email("user" + to_string(i) + "@example.com");
        user_service->create_user(user);
    }
    
    auto users = user_service->list_users(0, 10);
    ASSERT_EQUAL(users.size(), 5);
    
    // 测试分页
    auto first_page = user_service->list_users(0, 2);
    ASSERT_EQUAL(first_page.size(), 2);
    
    auto second_page = user_service->list_users(2, 2);
    ASSERT_EQUAL(second_page.size(), 2);
    
    cout << "✓ User list test passed" << endl;
    
    delete_temp_db(db_path);
}

int main() {
    cout << "Running UserService unit tests..." << endl;
    
    try {
        test_user_creation();
        test_user_authentication();
        test_user_update();
        test_user_deletion();
        test_user_list();
        
        cout << "\n✅ All UserService tests passed!" << endl;
        return 0;
    } catch (const exception& e) {
        cerr << "\n❌ Test failed: " << e.what() << endl;
        return 1;
    }
}
