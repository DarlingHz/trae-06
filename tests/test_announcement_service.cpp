#include <iostream>
#include <memory>
#include "../src/services/announcement_service.h"
#include "../src/repository/announcement_repository.h"
#include "../src/cache/cache_manager.h"
#include "test_utils.h"

using namespace std;

void test_announcement_creation() {
    cout << "\n=== Testing announcement creation ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto announcement_service = make_shared<AnnouncementService>(announcement_repo, cache_manager);
    
    Announcement announcement;
    announcement.set_title("Test Announcement");
    announcement.set_content("This is a test announcement content.");
    announcement.set_author_id(1);
    announcement.set_priority(AnnouncementPriority::PRIORITY_NORMAL);
    announcement.set_type(AnnouncementType::TYPE_INFORMATION);
    
    announcement_service->create_announcement(announcement);
    
    auto created_announcement = announcement_service->get_announcement_by_id(announcement.get_id());
    ASSERT_TRUE(created_announcement.has_value());
    ASSERT_EQUAL(created_announcement->get_title(), "Test Announcement");
    ASSERT_EQUAL(created_announcement->get_content(), "This is a test announcement content.");
    ASSERT_EQUAL(created_announcement->get_author_id(), 1);
    ASSERT_EQUAL(created_announcement->get_status(), AnnouncementStatus::STATUS_DRAFT);
    
    cout << "✓ Announcement creation test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_announcement_publishing() {
    cout << "\n=== Testing announcement publishing ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto announcement_service = make_shared<AnnouncementService>(announcement_repo, cache_manager);
    
    Announcement announcement;
    announcement.set_title("Test Announcement");
    announcement.set_content("This is a test announcement content.");
    announcement.set_author_id(1);
    announcement_service->create_announcement(announcement);
    
    // 发布公告
    announcement_service->publish_announcement(announcement.get_id());
    
    auto published_announcement = announcement_service->get_announcement_by_id(announcement.get_id());
    ASSERT_TRUE(published_announcement.has_value());
    ASSERT_EQUAL(published_announcement->get_status(), AnnouncementStatus::STATUS_PUBLISHED);
    
    // 取消发布公告
    announcement_service->unpublish_announcement(announcement.get_id());
    
    auto unpublished_announcement = announcement_service->get_announcement_by_id(announcement.get_id());
    ASSERT_TRUE(unpublished_announcement.has_value());
    ASSERT_EQUAL(unpublished_announcement->get_status(), AnnouncementStatus::STATUS_DRAFT);
    
    cout << "✓ Announcement publishing test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_announcement_update() {
    cout << "\n=== Testing announcement update ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto announcement_service = make_shared<AnnouncementService>(announcement_repo, cache_manager);
    
    Announcement announcement;
    announcement.set_title("Test Announcement");
    announcement.set_content("This is a test announcement content.");
    announcement.set_author_id(1);
    announcement_service->create_announcement(announcement);
    
    announcement.set_title("Updated Announcement");
    announcement.set_content("This is the updated content.");
    announcement.set_priority(AnnouncementPriority::PRIORITY_HIGH);
    announcement_service->update_announcement(announcement);
    
    auto updated_announcement = announcement_service->get_announcement_by_id(announcement.get_id());
    ASSERT_TRUE(updated_announcement.has_value());
    ASSERT_EQUAL(updated_announcement->get_title(), "Updated Announcement");
    ASSERT_EQUAL(updated_announcement->get_content(), "This is the updated content.");
    ASSERT_EQUAL(updated_announcement->get_priority(), AnnouncementPriority::PRIORITY_HIGH);
    
    cout << "✓ Announcement update test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_announcement_deletion() {
    cout << "\n=== Testing announcement deletion ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto announcement_service = make_shared<AnnouncementService>(announcement_repo, cache_manager);
    
    Announcement announcement;
    announcement.set_title("Test Announcement");
    announcement.set_content("This is a test announcement content.");
    announcement.set_author_id(1);
    announcement_service->create_announcement(announcement);
    
    announcement_service->delete_announcement(announcement.get_id());
    
    auto deleted_announcement = announcement_service->get_announcement_by_id(announcement.get_id());
    ASSERT_FALSE(deleted_announcement.has_value());
    
    cout << "✓ Announcement deletion test passed" << endl;
    
    delete_temp_db(db_path);
}

void test_announcement_list() {
    cout << "\n=== Testing announcement list ===" << endl;
    
    string db_path = create_temp_db_path();
    delete_temp_db(db_path);
    
    auto announcement_repo = make_shared<AnnouncementRepository>(db_path);
    auto cache_manager = make_shared<CacheManager>();
    auto announcement_service = make_shared<AnnouncementService>(announcement_repo, cache_manager);
    
    // 创建多个公告
    for (int i = 0; i < 5; ++i) {
        Announcement announcement;
        announcement.set_title("Announcement " + to_string(i));
        announcement.set_content("Content " + to_string(i));
        announcement.set_author_id(1);
        announcement_service->create_announcement(announcement);
        // 发布其中一些
        if (i % 2 == 0) {
            announcement_service->publish_announcement(announcement.get_id());
        }
    }
    
    // 测试获取所有公告
    auto all_announcements = announcement_service->list_announcements(AnnouncementFilter{}, 0, 10);
    ASSERT_EQUAL(all_announcements.size(), 5);
    
    // 测试只获取已发布的公告
    AnnouncementFilter published_filter;
    published_filter.status = AnnouncementStatus::STATUS_PUBLISHED;
    auto published_announcements = announcement_service->list_announcements(published_filter, 0, 10);
    ASSERT_EQUAL(published_announcements.size(), 3);  // 0, 2, 4 已发布
    
    // 测试分页
    auto first_page = announcement_service->list_announcements(AnnouncementFilter{}, 0, 2);
    ASSERT_EQUAL(first_page.size(), 2);
    
    cout << "✓ Announcement list test passed" << endl;
    
    delete_temp_db(db_path);
}

int main() {
    cout << "Running AnnouncementService unit tests..." << endl;
    
    try {
        test_announcement_creation();
        test_announcement_publishing();
        test_announcement_update();
        test_announcement_deletion();
        test_announcement_list();
        
        cout << "\n✅ All AnnouncementService tests passed!" << endl;
        return 0;
    } catch (const exception& e) {
        cerr << "\n❌ Test failed: " << e.what() << endl;
        return 1;
    }
}
