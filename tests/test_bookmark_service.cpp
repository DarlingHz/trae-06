#include <gtest/gtest.h>
#include <memory>
#include "service/UserService.h"
#include "service/BookmarkService.h"
#include "repository/DatabasePool.h"
#include "repository/UserRepository.h"
#include "repository/BookmarkRepository.h"
#include "auth/JWT.h"

class BookmarkServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use in-memory database for testing
        db_pool_ = std::make_shared<repository::DatabasePool>(":memory:", 2);
        user_repo_ = repository::create_user_repository(*db_pool_);
        bookmark_repo_ = repository::create_bookmark_repository(*db_pool_);
        
        jwt_ = std::make_shared<auth::JWT>("test_secret_key", 3600);
        user_service_ = std::make_shared<service::UserService>(std::move(user_repo_), jwt_);
        bookmark_service_ = std::make_shared<service::BookmarkService>(std::move(bookmark_repo_));

        // Create a test user
        service::UserRegisterRequest register_req;
        register_req.email = "test@example.com";
        register_req.password = "testpass123";
        register_req.nickname = "Test User";
        
        user_ = user_service_->register_user(register_req);
        ASSERT_NE(user_, nullptr);
        user_id_ = user_->id();
    }

    void TearDown() override {
        // Cleanup - no need to clear since we're using in-memory database
    }

    std::shared_ptr<repository::DatabasePool> db_pool_;
    std::unique_ptr<repository::UserRepository> user_repo_;
    std::unique_ptr<repository::BookmarkRepository> bookmark_repo_;
    std::shared_ptr<auth::JWT> jwt_;
    
    std::shared_ptr<service::UserService> user_service_;
    std::shared_ptr<service::BookmarkService> bookmark_service_;
    
    std::shared_ptr<models::User> user_;
    int user_id_;
};

TEST_F(BookmarkServiceTest, TestCreateBookmark) {
    service::BookmarkCreateRequest request;
    request.url = "https://www.example.com";
    request.title = "Example Website";
    request.description = "This is an example website";
    request.tags = {"test", "example"};
    request.folder = "test-folder";
    request.is_favorite = false;
    request.read_status = models::ReadStatus::UNREAD;

    auto bookmark = bookmark_service_->create_bookmark(user_id_, request);
    
    ASSERT_NE(bookmark, nullptr);
    ASSERT_EQ(bookmark->user_id(), user_id_);
    ASSERT_EQ(bookmark->url(), "https://www.example.com");
    ASSERT_EQ(bookmark->title(), "Example Website");
    ASSERT_EQ(bookmark->tags(), std::vector<std::string>({"test", "example"}));
    ASSERT_EQ(bookmark->folder(), "test-folder");
    ASSERT_FALSE(bookmark->is_favorite());
    ASSERT_EQ(bookmark->read_status(), models::ReadStatus::UNREAD);
}

TEST_F(BookmarkServiceTest, TestGetBookmark) {
    service::BookmarkCreateRequest create_req;
    create_req.url = "https://www.example.com";
    create_req.title = "Example Website";
    create_req.description = "Test";
    
    auto bookmark = bookmark_service_->create_bookmark(user_id_, create_req);
    ASSERT_NE(bookmark, nullptr);
    int bookmark_id = bookmark->id();

    auto retrieved_bookmark = bookmark_service_->get_bookmark(bookmark_id, user_id_);
    ASSERT_NE(retrieved_bookmark, nullptr);
    ASSERT_EQ(retrieved_bookmark->id(), bookmark_id);
    ASSERT_EQ(retrieved_bookmark->title(), "Example Website");
}

TEST_F(BookmarkServiceTest, TestUpdateBookmark) {
    service::BookmarkCreateRequest create_req;
    create_req.url = "https://www.example.com";
    create_req.title = "Example Website";
    create_req.description = "Test";
    
    auto bookmark = bookmark_service_->create_bookmark(user_id_, create_req);
    ASSERT_NE(bookmark, nullptr);
    int bookmark_id = bookmark->id();

    service::BookmarkUpdateRequest update_req;
    update_req.title = std::optional<std::string>("Updated Title");
    update_req.is_favorite = std::optional<bool>(true);
    update_req.read_status = std::optional<models::ReadStatus>(models::ReadStatus::READ);

    bool success = bookmark_service_->update_bookmark(bookmark_id, user_id_, update_req);
    ASSERT_TRUE(success);

    auto updated_bookmark = bookmark_service_->get_bookmark(bookmark_id, user_id_);
    ASSERT_NE(updated_bookmark, nullptr);
    ASSERT_EQ(updated_bookmark->title(), "Updated Title");
    ASSERT_TRUE(updated_bookmark->is_favorite());
    ASSERT_EQ(updated_bookmark->read_status(), models::ReadStatus::READ);
}

TEST_F(BookmarkServiceTest, TestDeleteBookmark) {
    service::BookmarkCreateRequest create_req;
    create_req.url = "https://www.example.com";
    create_req.title = "Example Website";
    
    auto bookmark = bookmark_service_->create_bookmark(user_id_, create_req);
    ASSERT_NE(bookmark, nullptr);
    int bookmark_id = bookmark->id();

    bool success = bookmark_service_->delete_bookmark(bookmark_id, user_id_);
    ASSERT_TRUE(success);

    auto retrieved_bookmark = bookmark_service_->get_bookmark(bookmark_id, user_id_);
    ASSERT_EQ(retrieved_bookmark, nullptr);
}

TEST_F(BookmarkServiceTest, TestQueryBookmarks) {
    // Create multiple bookmarks
    for(int i = 0; i < 5; ++i) {
        service::BookmarkCreateRequest create_req;
        create_req.url = "https://www.example" + std::to_string(i) + ".com";
        create_req.title = "Example " + std::to_string(i);
        create_req.description = "Test bookmark " + std::to_string(i);
        create_req.tags = {"test", "bookmark"};
        create_req.is_favorite = (i % 2 == 0);
        
        bookmark_service_->create_bookmark(user_id_, create_req);
    }

    // Test basic query
    service::BookmarkQueryRequest query_req;
    query_req.page = 1;
    query_req.page_size = 10;
    
    auto result = bookmark_service_->query_bookmarks(user_id_, query_req);
    ASSERT_EQ(result.total, 5);
    ASSERT_EQ(result.bookmarks.size(), 5);
}

TEST_F(BookmarkServiceTest, TestQueryWithFilter) {
    // Create bookmarks with different favorites
    service::BookmarkCreateRequest create_req1;
    create_req1.url = "https://www.example1.com";
    create_req1.title = "Favorite Bookmark";
    create_req1.is_favorite = true;
    bookmark_service_->create_bookmark(user_id_, create_req1);

    service::BookmarkCreateRequest create_req2;
    create_req2.url = "https://www.example2.com";
    create_req2.title = "Not Favorite Bookmark";
    create_req2.is_favorite = false;
    bookmark_service_->create_bookmark(user_id_, create_req2);

    // Query only favorites
    service::BookmarkQueryRequest query_req;
    query_req.is_favorite = std::optional<bool>(true);
    
    auto result = bookmark_service_->query_bookmarks(user_id_, query_req);
    ASSERT_EQ(result.total, 1);
    ASSERT_EQ(result.bookmarks.size(), 1);
    ASSERT_TRUE(result.bookmarks[0]->is_favorite());
}

TEST_F(BookmarkServiceTest, TestRenameTag) {
    service::BookmarkCreateRequest create_req;
    create_req.url = "https://www.example.com";
    create_req.title = "Example Website";
    create_req.tags = {"old-tag", "test"};
    
    auto bookmark = bookmark_service_->create_bookmark(user_id_, create_req);
    ASSERT_NE(bookmark, nullptr);
    int bookmark_id = bookmark->id();

    // Rename tag
    bool success = bookmark_service_->rename_tag(user_id_, "old-tag", "new-tag");
    ASSERT_TRUE(success);

    // Verify the tag was renamed
    auto updated_bookmark = bookmark_service_->get_bookmark(bookmark_id, user_id_);
    ASSERT_NE(updated_bookmark, nullptr);
    
    std::vector<std::string> expected_tags = {"new-tag", "test"};
    ASSERT_EQ(updated_bookmark->tags(), expected_tags);
}

TEST_F(BookmarkServiceTest, TestBatchUpdateReadStatus) {
    // Create multiple bookmarks
    std::vector<int> bookmark_ids;
    for(int i = 0; i < 3; ++i) {
        service::BookmarkCreateRequest create_req;
        create_req.url = "https://www.example" + std::to_string(i) + ".com";
        create_req.title = "Example " + std::to_string(i);
        create_req.read_status = models::ReadStatus::UNREAD;
        
        auto bookmark = bookmark_service_->create_bookmark(user_id_, create_req);
        ASSERT_NE(bookmark, nullptr);
        bookmark_ids.push_back(bookmark->id());
    }

    // Batch mark as read
    service::BatchUpdateRequest batch_req;
    batch_req.ids = bookmark_ids;
    
    bool success = bookmark_service_->batch_update_read_status(user_id_, batch_req, models::ReadStatus::READ);
    ASSERT_TRUE(success);

    // Verify all are read
    for(int id : bookmark_ids) {
        auto bookmark = bookmark_service_->get_bookmark(id, user_id_);
        ASSERT_NE(bookmark, nullptr);
        ASSERT_EQ(bookmark->read_status(), models::ReadStatus::READ);
    }
}

TEST_F(BookmarkServiceTest, TestUserStats) {
    // Create some bookmarks
    for(int i = 0; i < 5; ++i) {
        service::BookmarkCreateRequest create_req;
        create_req.url = "https://www.example" + std::to_string(i) + ".com";
        create_req.title = "Example " + std::to_string(i);
        create_req.read_status = (i < 3) ? models::ReadStatus::UNREAD : models::ReadStatus::READ;
        create_req.is_favorite = (i % 2 == 0);
        
        bookmark_service_->create_bookmark(user_id_, create_req);
    }

    auto stats = bookmark_service_->get_user_stats(user_id_);
    
    ASSERT_EQ(stats.total_bookmarks(), 5);
    ASSERT_EQ(stats.unread_count(), 3);
    ASSERT_EQ(stats.read_count(), 2);
    ASSERT_EQ(stats.favorite_count(), 3); // 0, 2, 4
}
