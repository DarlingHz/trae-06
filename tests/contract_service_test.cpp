#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <memory>
#include <string>
#include "service/contract_service.h"
#include "storage/sqlite_storage.h"

using namespace std;

// Helper function to create a temporary database
string create_temp_db() {
    return ":memory:";
}

TEST_CASE("ContractService - Create and submit contract", "[contract][create][submit]") {
    // Arrange
    string db_path = create_temp_db();
    auto storage = make_unique<storage::SQLiteStorage>(db_path);
    storage->init();
    
    // Insert test users
    domain::User user1;
    user1.name = "Test Employee";
    user1.department = "sales";
    user1.role = "employee";
    storage->create_user(user1);
    
    domain::User user2;
    user2.name = "Test Manager";
    user2.department = "sales";
    user2.role = "manager";
    storage->create_user(user2);
    
    auto service = make_unique<service::ContractService>(move(storage));
    
    // Act
    int user_id = 1;
    string title = "Test Contract";
    string counterparty = "Test Company";
    long long amount = 10000;
    string currency = "CNY";
    string department = "sales";
    
    auto contract = service->create_contract(user_id, title, counterparty, amount, currency, department);
    
    // Assert - contract created successfully
    REQUIRE(contract);
    REQUIRE(contract->id > 0);
    REQUIRE(contract->title == title);
    REQUIRE(contract->counterparty == counterparty);
    REQUIRE(contract->amount == amount);
    REQUIRE(contract->currency == currency);
    REQUIRE(contract->creator_id == user_id);
    REQUIRE(contract->department == department);
    REQUIRE(contract->status == domain::ContractStatus::DRAFT);
    
    // Act - submit contract
    auto submitted_contract = service->submit_contract(user_id, contract->id);
    
    // Assert - contract submitted successfully
    REQUIRE(submitted_contract);
    REQUIRE(submitted_contract->status == domain::ContractStatus::APPROVING);
    
    // Check approval steps were created
    auto progress = service->get_contract_approval_progress(contract->id);
    REQUIRE(progress);
    REQUIRE(progress->total_steps == 1); // Amount < 50000: only manager approval
    REQUIRE(progress->current_step == 1);
    REQUIRE(progress->current_role == "manager");
    REQUIRE(progress->current_approver_id == 2); // Test Manager has id 2
}

TEST_CASE("ContractService - Complete approval flow", "[contract][approval][approve]") {
    // Arrange
    string db_path = create_temp_db();
    auto storage = make_unique<storage::SQLiteStorage>(db_path);
    storage->init();
    
    // Insert test users
    domain::User employee;
    employee.name = "Test Employee";
    employee.department = "sales";
    employee.role = "employee";
    storage->create_user(employee);
    
    domain::User manager;
    manager.name = "Test Manager";
    manager.department = "sales";
    manager.role = "manager";
    storage->create_user(manager);
    
    domain::User finance_approver;
    finance_approver.name = "Test Finance Approver";
    finance_approver.department = "finance";
    finance_approver.role = "finance_approver";
    storage->create_user(finance_approver);
    
    auto service = make_unique<service::ContractService>(move(storage));
    
    // Create and submit contract (amount 100000, requires manager and finance approval)
    int user_id = 1;
    auto contract = service->create_contract(user_id, "Test Contract", "Test Company", 100000, "CNY", "sales");
    auto submitted_contract = service->submit_contract(user_id, contract->id);
    
    // Act - Manager approves
    int manager_id = 2;
    auto contract_after_manager_approval = service->approve_contract(manager_id, contract->id, "approve", "Approved by manager", nullopt);
    
    // Assert - contract is still approving, next step is finance
    REQUIRE(contract_after_manager_approval);
    REQUIRE(contract_after_manager_approval->status == domain::ContractStatus::APPROVING);
    
    auto progress1 = service->get_contract_approval_progress(contract->id);
    REQUIRE(progress1);
    REQUIRE(progress1->current_step == 2);
    REQUIRE(progress1->current_role == "finance_approver");
    REQUIRE(progress1->current_approver_id == 3);
    
    // Act - Finance approver approves
    int finance_id = 3;
    auto contract_after_finance_approval = service->approve_contract(finance_id, contract->id, "approve", "Approved by finance", nullopt);
    
    // Assert - contract is now approved
    REQUIRE(contract_after_finance_approval);
    REQUIRE(contract_after_finance_approval->status == domain::ContractStatus::APPROVED);
    
    auto progress2 = service->get_contract_approval_progress(contract->id);
    REQUIRE(progress2);
    REQUIRE(progress2->current_step == 2); // All steps completed
}

TEST_CASE("ContractService - Approval reject flow", "[contract][approval][reject]") {
    // Arrange
    string db_path = create_temp_db();
    auto storage = make_unique<storage::SQLiteStorage>(db_path);
    storage->init();
    
    // Insert test users
    domain::User employee;
    employee.name = "Test Employee";
    employee.department = "sales";
    employee.role = "employee";
    storage->create_user(employee);
    
    domain::User manager;
    manager.name = "Test Manager";
    manager.department = "sales";
    manager.role = "manager";
    storage->create_user(manager);
    
    auto service = make_unique<service::ContractService>(move(storage));
    
    // Create and submit contract
    int user_id = 1;
    auto contract = service->create_contract(user_id, "Test Contract", "Test Company", 50000, "CNY", "sales");
    auto submitted_contract = service->submit_contract(user_id, contract->id);
    
    // Act - Manager rejects
    int manager_id = 2;
    string reject_comment = "Rejected due to insufficient budget";
    auto contract_after_rejection = service->approve_contract(manager_id, contract->id, "reject", reject_comment, nullopt);
    
    // Assert - contract is rejected
    REQUIRE(contract_after_rejection);
    REQUIRE(contract_after_rejection->status == domain::ContractStatus::REJECTED);
    
    // Check approval log contains rejection
    auto history = service->get_approval_history(contract->id);
    REQUIRE_FALSE(history.empty());
    bool found_reject = false;
    for (const auto& log : history) {
        if (log.action == domain::ApprovalAction::REJECT && log.comment && *log.comment == reject_comment) {
            found_reject = true;
            break;
        }
    }
    REQUIRE(found_reject);
}

int main(int argc, char* argv[]) {
    int result = Catch::Session().run(argc, argv);
    return result;
}
