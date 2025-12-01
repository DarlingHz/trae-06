#include <gtest/gtest.h>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

// 测试服务器的地址和端口
const std::string TEST_SERVER_URL = "http://localhost:8080";

// 测试用的用户数据
const std::string TEST_USER_NAME_1 = "alice";
const std::string TEST_USER_NAME_2 = "bob";
const std::string TEST_USER_NAME_3 = "charlie";

// 测试用的会话数据
const std::string TEST_CONVERSATION_TITLE = "项目讨论";

// 测试用的消息数据
const std::string TEST_MESSAGE_CONTENT_1 = "大家好，今天我们来讨论一下新项目的进展情况。";
const std::string TEST_MESSAGE_CONTENT_2 = "好的，我已经完成了项目的需求分析文档。";
const std::string TEST_MESSAGE_CONTENT_3 = "我正在开发项目的核心功能模块。";
const std::string TEST_MESSAGE_CONTENT_4 = "我已经完成了项目的UI设计稿。";
const std::string TEST_MESSAGE_CONTENT_UPDATED = "大家好，今天我们来讨论一下新项目的进展情况和下一步计划。";

// 测试用的搜索关键词
const std::string TEST_SEARCH_KEYWORD = "项目";

// 辅助函数：发送HTTP请求
json send_http_request(const std::string& method, const std::string& path, 
                        const json& request_body = json::object()) {
    httplib::Client client(TEST_SERVER_URL);
    client.set_timeout_sec(10);
    
    httplib::Result res;
    
    if (method == "GET") {
        res = client.Get(path.c_str());
    } else if (method == "POST") {
        std::string body = request_body.dump();
        res = client.Post(path.c_str(), body, "application/json");
    } else if (method == "PUT") {
        std::string body = request_body.dump();
        res = client.Put(path.c_str(), body, "application/json");
    } else if (method == "DELETE") {
        res = client.Delete(path.c_str());
    } else {
        throw std::invalid_argument("Unsupported HTTP method: " + method);
    }
    
    if (!res) {
        throw std::runtime_error("HTTP request failed: " + std::string(res.error().message()));
    }
    
    if (res->status != 200) {
        throw std::runtime_error("HTTP request returned error status: " + std::to_string(res->status) + 
                                   ", body: " + res->body);
    }
    
    return json::parse(res->body);
}

// 测试类：ChatArchiveTest
class ChatArchiveTest : public ::testing::Test {
protected:
    // 测试前的准备工作
    void SetUp() override {
        // 清空测试数据（可选，根据实际情况而定）
        // 注意：在实际测试环境中，应该使用专门的测试数据库，并在测试前清空数据
    }
    
    // 测试后的清理工作
    void TearDown() override {
        // 清空测试数据（可选，根据实际情况而定）
    }
    
    // 测试数据
    std::vector<int64_t> test_user_ids;
    int64_t test_conversation_id = 0;
    std::vector<int64_t> test_message_ids;
};

// 测试用例：创建用户
TEST_F(ChatArchiveTest, CreateUsers) {
    // 创建第一个测试用户
    json create_user_request_1;
    create_user_request_1["name"] = TEST_USER_NAME_1;
    
    json create_user_response_1 = send_http_request("POST", "/api/users", create_user_request_1);
    
    EXPECT_EQ(create_user_response_1["data"]["name"], TEST_USER_NAME_1);
    EXPECT_GT(create_user_response_1["data"]["id"], 0);
    
    test_user_ids.push_back(create_user_response_1["data"]["id"]);
    
    // 创建第二个测试用户
    json create_user_request_2;
    create_user_request_2["name"] = TEST_USER_NAME_2;
    
    json create_user_response_2 = send_http_request("POST", "/api/users", create_user_request_2);
    
    EXPECT_EQ(create_user_response_2["data"]["name"], TEST_USER_NAME_2);
    EXPECT_GT(create_user_response_2["data"]["id"], 0);
    
    test_user_ids.push_back(create_user_response_2["data"]["id"]);
    
    // 创建第三个测试用户
    json create_user_request_3;
    create_user_request_3["name"] = TEST_USER_NAME_3;
    
    json create_user_response_3 = send_http_request("POST", "/api/users", create_user_request_3);
    
    EXPECT_EQ(create_user_response_3["data"]["name"], TEST_USER_NAME_3);
    EXPECT_GT(create_user_response_3["data"]["id"], 0);
    
    test_user_ids.push_back(create_user_response_3["data"]["id"]);
    
    // 验证用户数量
    json get_users_response = send_http_request("GET", "/api/users?limit=10&offset=0");
    
    EXPECT_GE(get_users_response["data"].size(), 3);
}

// 测试用例：创建会话
TEST_F(ChatArchiveTest, CreateConversation) {
    // 确保已经创建了测试用户
    if (test_user_ids.empty()) {
        CreateUsers();
    }
    
    // 创建测试会话
    json create_conversation_request;
    create_conversation_request["title"] = TEST_CONVERSATION_TITLE;
    create_conversation_request["participant_ids"] = test_user_ids;
    
    json create_conversation_response = send_http_request("POST", "/api/conversations", create_conversation_request);
    
    EXPECT_EQ(create_conversation_response["data"]["title"], TEST_CONVERSATION_TITLE);
    EXPECT_GT(create_conversation_response["data"]["id"], 0);
    
    test_conversation_id = create_conversation_response["data"]["id"];
    
    // 验证会话数量
    json get_conversations_response = send_http_request("GET", "/api/conversations?limit=10&offset=0");
    
    EXPECT_GE(get_conversations_response["data"].size(), 1);
}

// 测试用例：向会话添加消息
TEST_F(ChatArchiveTest, AddMessagesToConversation) {
    // 确保已经创建了测试用户和会话
    if (test_user_ids.empty()) {
        CreateUsers();
    }
    
    if (test_conversation_id == 0) {
        CreateConversation();
    }
    
    // 向会话添加第一条消息
    json create_message_request_1;
    create_message_request_1["sender_id"] = test_user_ids[0];
    create_message_request_1["content"] = TEST_MESSAGE_CONTENT_1;
    
    std::string create_message_path_1 = "/api/conversations/" + std::to_string(test_conversation_id) + "/messages";
    json create_message_response_1 = send_http_request("POST", create_message_path_1, create_message_request_1);
    
    EXPECT_EQ(create_message_response_1["data"]["content"], TEST_MESSAGE_CONTENT_1);
    EXPECT_EQ(create_message_response_1["data"]["sender_id"], test_user_ids[0]);
    EXPECT_EQ(create_message_response_1["data"]["conversation_id"], test_conversation_id);
    EXPECT_GT(create_message_response_1["data"]["id"], 0);
    
    test_message_ids.push_back(create_message_response_1["data"]["id"]);
    
    // 向会话添加第二条消息
    json create_message_request_2;
    create_message_request_2["sender_id"] = test_user_ids[1];
    create_message_request_2["content"] = TEST_MESSAGE_CONTENT_2;
    
    std::string create_message_path_2 = "/api/conversations/" + std::to_string(test_conversation_id) + "/messages";
    json create_message_response_2 = send_http_request("POST", create_message_path_2, create_message_request_2);
    
    EXPECT_EQ(create_message_response_2["data"]["content"], TEST_MESSAGE_CONTENT_2);
    EXPECT_EQ(create_message_response_2["data"]["sender_id"], test_user_ids[1]);
    EXPECT_EQ(create_message_response_2["data"]["conversation_id"], test_conversation_id);
    EXPECT_GT(create_message_response_2["data"]["id"], 0);
    
    test_message_ids.push_back(create_message_response_2["data"]["id"]);
    
    // 向会话添加第三条消息
    json create_message_request_3;
    create_message_request_3["sender_id"] = test_user_ids[2];
    create_message_request_3["content"] = TEST_MESSAGE_CONTENT_3;
    
    std::string create_message_path_3 = "/api/conversations/" + std::to_string(test_conversation_id) + "/messages";
    json create_message_response_3 = send_http_request("POST", create_message_path_3, create_message_request_3);
    
    EXPECT_EQ(create_message_response_3["data"]["content"], TEST_MESSAGE_CONTENT_3);
    EXPECT_EQ(create_message_response_3["data"]["sender_id"], test_user_ids[2]);
    EXPECT_EQ(create_message_response_3["data"]["conversation_id"], test_conversation_id);
    EXPECT_GT(create_message_response_3["data"]["id"], 0);
    
    test_message_ids.push_back(create_message_response_3["data"]["id"]);
    
    // 向会话添加第四条消息
    json create_message_request_4;
    create_message_request_4["sender_id"] = test_user_ids[0];
    create_message_request_4["content"] = TEST_MESSAGE_CONTENT_4;
    
    std::string create_message_path_4 = "/api/conversations/" + std::to_string(test_conversation_id) + "/messages";
    json create_message_response_4 = send_http_request("POST", create_message_path_4, create_message_request_4);
    
    EXPECT_EQ(create_message_response_4["data"]["content"], TEST_MESSAGE_CONTENT_4);
    EXPECT_EQ(create_message_response_4["data"]["sender_id"], test_user_ids[0]);
    EXPECT_EQ(create_message_response_4["data"]["conversation_id"], test_conversation_id);
    EXPECT_GT(create_message_response_4["data"]["id"], 0);
    
    test_message_ids.push_back(create_message_response_4["data"]["id"]);
    
    // 验证会话中的消息数量
    std::string get_messages_path = "/api/conversations/" + std::to_string(test_conversation_id) + "/messages?limit=10&offset=0";
    json get_messages_response = send_http_request("GET", get_messages_path);
    
    EXPECT_EQ(get_messages_response["data"].size(), 4);
}

// 测试用例：搜索消息
TEST_F(ChatArchiveTest, SearchMessages) {
    // 确保已经创建了测试用户、会话和消息
    if (test_user_ids.empty()) {
        CreateUsers();
    }
    
    if (test_conversation_id == 0) {
        CreateConversation();
    }
    
    if (test_message_ids.empty()) {
        AddMessagesToConversation();
    }
    
    // 搜索包含关键词"项目"的消息
    std::string search_messages_path = "/api/search/messages?keyword=" + TEST_SEARCH_KEYWORD + "&limit=10&offset=0";
    json search_messages_response = send_http_request("GET", search_messages_path);
    
    EXPECT_GE(search_messages_response["data"]["total_count"], 4);
    EXPECT_GE(search_messages_response["data"]["messages"].size(), 4);
    
    // 验证搜索结果中的消息都包含关键词"项目"
    for (const auto& message : search_messages_response["data"]["messages"]) {
        std::string content = message["content"];
        EXPECT_NE(content.find(TEST_SEARCH_KEYWORD), std::string::npos);
    }
}

// 测试用例：更新消息
TEST_F(ChatArchiveTest, UpdateMessage) {
    // 确保已经创建了测试用户、会话和消息
    if (test_user_ids.empty()) {
        CreateUsers();
    }
    
    if (test_conversation_id == 0) {
        CreateConversation();
    }
    
    if (test_message_ids.empty()) {
        AddMessagesToConversation();
    }
    
    // 更新第一条消息
    json update_message_request;
    update_message_request["content"] = TEST_MESSAGE_CONTENT_UPDATED;
    
    std::string update_message_path = "/api/messages/" + std::to_string(test_message_ids[0]);
    json update_message_response = send_http_request("PUT", update_message_path, update_message_request);
    
    EXPECT_EQ(update_message_response["data"]["content"], TEST_MESSAGE_CONTENT_UPDATED);
    EXPECT_EQ(update_message_response["data"]["id"], test_message_ids[0]);
    
    // 验证消息已经更新
    std::string get_message_path = "/api/messages/" + std::to_string(test_message_ids[0]);
    json get_message_response = send_http_request("GET", get_message_path);
    
    EXPECT_EQ(get_message_response["data"]["content"], TEST_MESSAGE_CONTENT_UPDATED);
    EXPECT_NE(get_message_response["data"].find("edited_at"), get_message_response["data"].end());
}

// 测试用例：删除消息
TEST_F(ChatArchiveTest, DeleteMessage) {
    // 确保已经创建了测试用户、会话和消息
    if (test_user_ids.empty()) {
        CreateUsers();
    }
    
    if (test_conversation_id == 0) {
        CreateConversation();
    }
    
    if (test_message_ids.empty()) {
        AddMessagesToConversation();
    }
    
    // 删除第二条消息
    std::string delete_message_path = "/api/messages/" + std::to_string(test_message_ids[1]);
    json delete_message_response = send_http_request("DELETE", delete_message_path);
    
    EXPECT_EQ(delete_message_response["data"]["id"], test_message_ids[1]);
    EXPECT_EQ(delete_message_response["data"]["deleted"], true);
    
    // 验证消息已经被标记为删除
    std::string get_message_path = "/api/messages/" + std::to_string(test_message_ids[1]);
    json get_message_response = send_http_request("GET", get_message_path);
    
    // 注意：根据业务逻辑，软删除的消息可能仍然可以获取，或者返回404
    // 这里假设软删除的消息仍然可以获取，但会有一个deleted字段标记为true
    // 如果业务逻辑是软删除的消息返回404，那么这里应该修改为验证返回404
    EXPECT_EQ(get_message_response["data"]["id"], test_message_ids[1]);
    EXPECT_EQ(get_message_response["data"]["deleted"], true);
    
    // 验证会话中的消息数量（软删除的消息默认不返回）
    std::string get_messages_path = "/api/conversations/" + std::to_string(test_conversation_id) + "/messages?limit=10&offset=0";
    json get_messages_response = send_http_request("GET", get_messages_path);
    
    EXPECT_EQ(get_messages_response["data"].size(), 3);
}

// 测试用例：获取统计信息
TEST_F(ChatArchiveTest, GetStatsOverview) {
    // 确保已经创建了测试用户、会话和消息
    if (test_user_ids.empty()) {
        CreateUsers();
    }
    
    if (test_conversation_id == 0) {
        CreateConversation();
    }
    
    if (test_message_ids.empty()) {
        AddMessagesToConversation();
    }
    
    // 获取统计信息
    json get_stats_response = send_http_request("GET", "/api/stats/overview");
    
    EXPECT_GE(get_stats_response["data"]["total_users"], 3);
    EXPECT_GE(get_stats_response["data"]["total_conversations"], 1);
    EXPECT_GE(get_stats_response["data"]["total_messages"], 4);
    EXPECT_GE(get_stats_response["data"]["messages_last_24h"], 4);
    EXPECT_GE(get_stats_response["data"]["top_senders"].size(), 3);
}

// 主函数：运行所有测试用例
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}