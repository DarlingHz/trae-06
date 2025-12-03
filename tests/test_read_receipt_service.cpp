#include <iostream>
#include <memory>
#include "../src/services/read_receipt_service.h"
#include "../src/repository/read_receipt_repository.h"
#include "../src/repository/announcement_repository.h"
#include "../src/repository/user_repository.h"
#include "../src/cache/cache_manager.h"
#include "test_utils.h"

using namespace std;

void test_read_receipt_creation() {
    cout << "\n=== Testing read receipt creation ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto read_receipt_repo = make_shared<ReadReceiptRepository>(db_path);
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto read_receipt_service = make_shared<ReadReceiptService>(
        read_receipt_repo, announcement_repo, user_repo, cache_manager
    );
    
    // 创建测试用户和公告
    User user;
    user.set_username("testuser");
    user.set_password("password123");
    user.set_email("test@example.com");
    user_repo->create(user);
    
    Announcement announcement;
    announcement.set_title("Test Announcement");
    announcement.set_content("Test content");
    announcement.set_author_id(user.get_id());
    announcement_repo->create(announcement);
    
    // 标记已读
    read_receipt_service->mark_as_read(user.get_id(), announcement.get_id());
    
    // 检查阅读记录
    auto receipt = read_receipt_service->get_read_receipt(user.get_id(), announcement.get_id());
    ASSERT_TRUE(receipt.has_value());
    ASSERT_EQUAL(receipt->get_user_id(), user.get_id());
    ASSERT_EQUAL(receipt->get_announcement_id(), announcement.get_id());
    
    // 检查用户公告是否已读
    bool is_read = read_receipt_service->is_announcement_read(user.get_id(), announcement.get_id());
    ASSERT_TRUE(is_read);
    
    cout << "✓ Read receipt creation test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_unread_announcements() {
    cout << "\n=== Testing unread announcements ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto read_receipt_repo = make_shared<ReadReceiptRepository>(db_path);
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto read_receipt_service = make_shared<ReadReceiptService>(
        read_receipt_repo, announcement_repo, user_repo, cache_manager
    );
    
    // 创建测试用户
    User user;
    user.set_username("testuser");
    user.set_password("password123");
    user.set_email("test@example.com");
    user_repo->create(user);
    
    // 创建多个公告
    vector<Announcement> announcements;
    for (int i = 0; i < 5; ++i) {
        Announcement announcement;
        announcement.set_title("Announcement " + to_string(i));
        announcement.set_content("Content " + to_string(i));
        announcement.set_author_id(user.get_id());
        announcement_repo->create(announcement);
        announcements.push_back(announcement);
    }
    
    // 标记前2个为已读
    read_receipt_service->mark_as_read(user.get_id(), announcements[0].get_id());
    read_receipt_service->mark_as_read(user.get_id(), announcements[1].get_id());
    
    // 获取未读公告数量
    size_t unread_count = read_receipt_service->get_unread_count(user.get_id());
    ASSERT_EQUAL(unread_count, 3);
    
    // 获取未读公告列表
    ReadReceiptFilter filter;
    filter.user_id = user.get_id();
    filter.read_status = ReadStatus::UNREAD;
    auto unread_announcements = read_receipt_service->list_unread_announcements(user.get_id(), 0, 10);
    ASSERT_EQUAL(unread_announcements.size(), 3);
    
    cout << "✓ Unread announcements test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_batch_mark_as_read() {
    cout << "\n=== Testing batch mark as read ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto read_receipt_repo = make_shared<ReadReceiptRepository>(db_path);
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto read_receipt_service = make_shared<ReadReceiptService>(
        read_receipt_repo, announcement_repo, user_repo, cache_manager
    );
    
    // 创建测试用户
    User user;
    user.set_username("testuser");
    user.set_password("password123");
    user.set_email("test@example.com");
    user_repo->create(user);
    
    // 创建多个公告
    vector<int64_t> announcement_ids;
    for (int i = 0; i < 5; ++i) {
        Announcement announcement;
        announcement.set_title("Announcement " + to_string(i));
        announcement.set_content("Content " + to_string(i));
        announcement.set_author_id(user.get_id());
        announcement_repo->create(announcement);
        announcement_ids.push_back(announcement.get_id());
    }
    
    // 批量标记为已读
    read_receipt_service->mark_multiple_as_read(user.get_id(), announcement_ids);
    
    // 检查所有公告是否都已读
    for (int64_t ann_id : announcement_ids) {
        bool is_read = read_receipt_service->is_announcement_read(user.get_id(), ann_id);
        ASSERT_TRUE(is_read);
    }
    
    // 检查未读数量
    size_t unread_count = read_receipt_service->get_unread_count(user.get_id());
    ASSERT_EQUAL(unread_count, 0);
    
    cout << "✓ Batch mark as read test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_read_receipt_statistics() {
    cout << "\n=== Testing read receipt statistics ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto read_receipt_repo = make_shared<ReadReceiptRepository>(db_path);
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto user_repo = make_shared<UserRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto read_receipt_service = make_shared<ReadReceiptService>(
        read_receipt_repo, announcement_repo, user_repo, cache_manager
    );
    
    // 创建测试用户和管理员
    User user1;
    user1.set_username("user1");
    user1.set_password("password1");
    user1.set_email("user1@example.com");
    user_repo->create(user1);
    
    User user2;
    user2.set_username("user2");
    user2.set_password("password2");
    user2.set_email("user2@example.com");
    user_repo->create(user2);
    
    User admin;
    admin.set_username("admin");
    admin.set_password("adminpass");
    admin.set_email("admin@example.com");
    admin.set_role(UserRole::ROLE_ADMIN);
    user_repo->create(admin);
    
    // 创建公告
    Announcement announcement;
    announcement.set_title("Test Announcement");
    announcement.set_content("Test content");
    announcement.set_author_id(admin.get_id());
    announcement_repo->create(announcement);
    
    // 让两个用户都读取
    read_receipt_service->mark_as_read(user1.get_id(), announcement.get_id());
    read_receipt_service->mark_as_read(user2.get_id(), announcement.get_id());
    
    // 获取统计信息
    auto stats = read_receipt_service->get_announcement_statistics(announcement.get_id());
    
    cout << "  Total users: " << stats.total_users << endl;
    cout << "  Read count: " << stats.read_count << endl;
    cout << "  Unread count: " << stats.unread_count << endl;
    cout << "  Read rate: " << stats.read_rate << "%" << endl;
    
    ASSERT_EQUAL(stats.total_users, 3);  // 包括管理员
    ASSERT_EQUAL(stats.read_count, 2);
    ASSERT_EQUAL(stats.unread_count, 1);
    ASSERT_TRUE(stats.read_rate > 66.6 && stats.read_rate < 66.7);
    
    cout << "✓ Read receipt statistics test passed" << endl;
    
    delete_temp_db(db_path);
}

int main() {
    cout << "Running ReadReceiptService unit tests..." << endl;
    
    try {
        test_read_receipt_creation();
        test_unread_announcements();
        test_batch_mark_as_read();
        test_read_receipt_statistics();
        
        cout << "\n✅ All ReadReceiptService tests passed!" << endl;
        return 0;
    } catch (const exception& e) {
        cerr << "\n❌ Test failed: " << e.what() << endl;
        return 1;
    }
}
