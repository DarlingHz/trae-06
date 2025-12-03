#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cassert>

#include "../src/utils/date_utils.h"
#include "../src/utils/logger.h"
#include "../src/models/user.h"
#include "../src/models/device.h"
#include "../src/models/warranty_policy.h"
#include "../src/models/service_center.h"
#include "../src/models/repair_order.h"
#include "../src/models/repair_status_history.h"
#include "../src/utils/json_utils.h"

using namespace std;

// 测试工具类
void testUtils() {
    cout << "Testing utilities..." << endl;
    
    // 测试日期工具
    string today = DateUtils::getToday();
    cout << "Today: " << today << endl;
    assert(!today.empty());
    
    string tomorrow = DateUtils::addDays(today, 1);
    cout << "Tomorrow: " << tomorrow << endl;
    assert(tomorrow > today);
    
    string yesterday = DateUtils::addDays(today, -1);
    cout << "Yesterday: " << yesterday << endl;
    assert(yesterday < today);
    
    // 测试日期比较
    assert(DateUtils::isBefore(yesterday, today));
    assert(DateUtils::isAfter(tomorrow, today));
    assert(DateUtils::isSameDay(today, today));
    
    // 测试日志工具
    Logger::init(Logger::Level::DEBUG, true);
    LOG_DEBUG("Debug test message");
    LOG_INFO("Info test message");
    LOG_WARNING("Warning test message");
    LOG_ERROR("Error test message");
    
    cout << "Utilities tests passed!" << endl << endl;
}

// 测试用户模型
void testUserModel() {
    cout << "Testing User model..." << endl;
    
    // 创建用户
    User user("张三", "zhangsan@example.com");
    cout << "Created user: " << user.name << ", " << user.email << endl;
    
    // 验证用户
    assert(user.isValid());
    
    // 测试无效用户
    User invalidUser("", "invalid-email");
    assert(!invalidUser.isValid());
    
    cout << "User model tests passed!" << endl << endl;
}

// 测试设备模型
void testDeviceModel() {
    cout << "Testing Device model..." << endl;
    
    // 创建设备
    Device device(1, "A1234567890", "Apple", "iPhone 14 Pro", "2023-01-15", "2024-01-15");
    device.category = Device::Category::Phone;
    cout << "Created device: " << device.brand << " " << device.model << endl;
    
    // 验证设备
    assert(device.isValid());
    
    // 测试保修状态
    assert(device.isUnderWarranty());
    
    // 测试类别转换
    string catStr = Device::categoryToString(Device::Category::Phone);
    assert(catStr == "Phone");
    
    Device::Category cat = Device::categoryFromString("Laptop");
    assert(cat == Device::Category::Laptop);
    
    cout << "Device model tests passed!" << endl << endl;
}

// 测试保修策略模型
void testWarrantyPolicyModel() {
    cout << "Testing WarrantyPolicy model..." << endl;
    
    // 创建保修策略
    WarrantyPolicy policy(1, WarrantyPolicy::PolicyType::Manufacturer, "Basic coverage", "Apple Inc.", "2023-01-15", "2024-01-15");
    cout << "Created policy: " << WarrantyPolicy::policyTypeToString(policy.policyType) << endl;
    
    // 验证保修策略
    assert(policy.isValid());
    
    // 测试策略类型转换
    string typeStr = WarrantyPolicy::policyTypeToString(WarrantyPolicy::PolicyType::Extended);
    assert(typeStr == "Extended");
    
    WarrantyPolicy::PolicyType type = WarrantyPolicy::policyTypeFromString("Accidental");
    assert(type == WarrantyPolicy::PolicyType::Accidental);
    
    cout << "WarrantyPolicy model tests passed!" << endl << endl;
}

// 测试维修网点模型
void testServiceCenterModel() {
    cout << "Testing ServiceCenter model..." << endl;
    
    // 创建维修网点
    ServiceCenter center("Apple Store", "Beijing", "101 Wangfujing Street", "400-666-8800", "beijing@apple.com");
    cout << "Created service center: " << center.name << ", " << center.city << endl;
    
    // 验证维修网点
    assert(center.isValid());
    
    cout << "ServiceCenter model tests passed!" << endl << endl;
}

// 测试维修订单模型
void testRepairOrderModel() {
    cout << "Testing RepairOrder model..." << endl;
    
    // 创建维修订单
    RepairOrder order(1, 1, 1, "屏幕碎裂", RepairOrder::Status::PendingReview, 2000.00);
    cout << "Created repair order: " << order.issueDescription << endl;
    
    // 验证维修订单
    assert(order.isValid());
    
    // 测试状态转换
    string statusStr = RepairOrder::statusToString(RepairOrder::Status::Accepted);
    assert(statusStr == "Accepted");
    
    RepairOrder::Status status = RepairOrder::statusFromString("Repaired");
    assert(status == RepairOrder::Status::Repaired);
    
    cout << "RepairOrder model tests passed!" << endl << endl;
}

// 测试维修状态历史模型
void testRepairStatusHistoryModel() {
    cout << "Testing RepairStatusHistory model..." << endl;
    
    // 创建维修状态历史
    RepairStatusHistory history(1, RepairOrder::Status::PendingReview, "等待审核", "系统自动");
    cout << "Created repair status history: " << history.changedBy << endl;
    
    // 验证维修状态历史
    assert(history.isValid());
    
    cout << "RepairStatusHistory model tests passed!" << endl << endl;
}

// 测试JSON工具
void testJsonUtils() {
    cout << "Testing JSON utilities..." << endl;
    
    // 创建JSON对象
    JsonObject obj;
    obj.set("name", "张三");
    obj.set("age", 30);
    obj.set("email", "zhangsan@example.com");
    obj.set("active", true);
    
    cout << "Created JSON: " << obj.toString() << endl;
    
    // 测试JSON数组
    JsonArray arr;
    arr.addString("Apple");
    arr.addString("Samsung");
    arr.addString("Huawei");
    obj.set("devices", arr);
    
    cout << "With array: " << obj.toString() << endl;
    
    // 测试解析JSON
    string jsonStr = R"({"name":"李四","age":25,"email":"lisi@example.com"})";
    JsonValue json = parseJson(jsonStr);
    assert(json.getType() == JsonValue::Type::Object);
    
    JsonObject parsedObj = json.asObject();
    assert(parsedObj.has("name"));
    assert(parsedObj["name"].asString() == "李四");
    assert(parsedObj["age"].asInt() == 25);
    assert(parsedObj["email"].asString() == "lisi@example.com");
    
    cout << "JSON parsing test passed!" << endl;
    
    cout << "JSON utilities tests passed!" << endl << endl;
}

// 测试配置工具
void testConfig() {
    cout << "Testing Config..." << endl;
    
    Config& config = Config::getInstance();
    
    // 测试设置和获取配置
    config.set("TEST_KEY", "TEST_VALUE");
    assert(config.get("TEST_KEY") == "TEST_VALUE");
    
    // 测试默认值
    assert(config.get("NON_EXISTENT_KEY", "default") == "default");
    
    // 测试整数配置
    config.set("TEST_INT", "42");
    assert(stoi(config.get("TEST_INT")) == 42);
    
    cout << "Config tests passed!" << endl << endl;
}

int main() {
    try {
        cout << "=== Starting Unit Tests ===" << endl << endl;
        
        testUtils();
        testUserModel();
        testDeviceModel();
        testWarrantyPolicyModel();
        testServiceCenterModel();
        testRepairOrderModel();
        testRepairStatusHistoryModel();
        testJsonUtils();
        testConfig();
        
        cout << "=== All Tests Passed Successfully! ===" << endl;
        return 0;
    } catch (const exception& e) {
        cout << "=== Test Failed: " << e.what() << " ===" << endl;
        return 1;
    }
}
