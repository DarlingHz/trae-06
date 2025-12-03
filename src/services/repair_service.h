#pragma once

#include <memory>
#include <vector>
#include <stdexcept>
#include <map>
#include "../models/repair_order.h"
#include "../models/repair_status_history.h"
#include "../repositories/repair_order_repository.h"
#include "../repositories/repair_status_history_repository.h"
#include "device_service.h"
#include "service_center_service.h"
#include "user_service.h"
#include "../utils/logger.h"
#include "../utils/date_utils.h"

class RepairService {
public:
    std::shared_ptr<RepairOrder> createRepairOrder(int userId, int deviceId, int serviceCenterId,
        const std::string& problemDescription, const std::string& expectedFinishDate = "") {
        // 验证用户存在
        UserService& userService = UserService::getInstance();
        if (!userService.userExists(userId)) {
            throw std::runtime_error("User not found");
        }

        // 验证设备存在
        DeviceService& deviceService = DeviceService::getInstance();
        if (!deviceService.deviceExists(deviceId)) {
            throw std::runtime_error("Device not found");
        }

        // 验证维修网点存在
        ServiceCenterService& scService = ServiceCenterService::getInstance();
        if (!scService.serviceCenterExists(serviceCenterId)) {
            throw std::runtime_error("Service center not found");
        }

        RepairOrder repairOrder;
        repairOrder.userId = userId;
        repairOrder.deviceId = deviceId;
        repairOrder.serviceCenterId = serviceCenterId;
        repairOrder.status = RepairOrder::Status::PendingReview;
        repairOrder.problemDescription = problemDescription;
        repairOrder.expectedFinishDate = expectedFinishDate;

        if (!repairOrder.isValid()) {
            throw std::runtime_error("Invalid repair order data");
        }

        try {
            auto createdOrder = RepairOrderRepository::getInstance().create(repairOrder);
            
            // 创建状态历史记录
            RepairStatusHistory history;
            history.repairOrderId = createdOrder->id;
            history.status = RepairOrder::statusToString(RepairOrder::Status::PendingReview);
            history.note = "Order created";
            history.operatorUser = "system";
            
            RepairStatusHistoryRepository::getInstance().create(history);
            
            return createdOrder;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to create repair order: %s", e.what());
            throw;
        }
    }

    std::shared_ptr<RepairOrder> getRepairOrderById(int id) {
        if (id <= 0) {
            throw std::runtime_error("Invalid repair order ID");
        }

        try {
            auto repairOrder = RepairOrderRepository::getInstance().findById(id);
            if (!repairOrder) {
                throw std::runtime_error("Repair order not found");
            }
            return repairOrder;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get repair order by ID: %s", e.what());
            throw;
        }
    }

    std::vector<std::shared_ptr<RepairOrder>> getRepairOrders(int userId = 0, int serviceCenterId = 0,
        const std::string& status = "", const std::string& city = "", const std::string& startDate = "",
        const std::string& endDate = "", const std::string& sortBy = "created_at", bool ascending = true,
        int page = 1, int pageSize = 10) {
        try {
            return RepairOrderRepository::getInstance().findByFilters(userId, serviceCenterId, status, city,
                startDate, endDate, sortBy, ascending, page, pageSize);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get repair orders: %s", e.what());
            throw;
        }
    }

    std::shared_ptr<RepairOrder> updateRepairOrderStatus(int id, const std::string& newStatusStr, 
        const std::string& note = "", const std::string& operatorUser = "system") {
        if (id <= 0) {
            throw std::runtime_error("Invalid repair order ID");
        }

        // 验证维修单存在
        auto repairOrder = getRepairOrderById(id);
        if (!repairOrder) {
            throw std::runtime_error("Repair order not found");
        }

        // 验证状态合法性
        RepairOrder::Status newStatus = RepairOrder::statusFromString(newStatusStr);
        if (newStatus == RepairOrder::Status::Unknown) {
            throw std::runtime_error("Invalid status value");
        }

        // 检查状态是否有变化
        if (repairOrder->status == newStatus) {
            return repairOrder; // 状态未变化，直接返回
        }

        try {
            // 更新维修单状态
            auto updatedOrder = RepairOrderRepository::getInstance().updateStatus(id, newStatus, note);
            
            // 创建状态历史记录
            RepairStatusHistory history;
            history.repairOrderId = id;
            history.status = newStatusStr;
            history.note = note;
            history.operatorUser = operatorUser;
            
            RepairStatusHistoryRepository::getInstance().create(history);
            
            return updatedOrder;
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to update repair order status: %s", e.what());
            throw;
        }
    }

    std::vector<std::shared_ptr<RepairStatusHistory>> getRepairOrderHistory(int repairOrderId) {
        if (repairOrderId <= 0) {
            throw std::runtime_error("Invalid repair order ID");
        }

        // 验证维修单存在
        if (!getRepairOrderById(repairOrderId)) {
            throw std::runtime_error("Repair order not found");
        }

        try {
            return RepairStatusHistoryRepository::getInstance().findByRepairOrderId(repairOrderId);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get repair order history: %s", e.what());
            throw;
        }
    }

    std::map<std::string, int> getRepairStatusStatistics() {
        try {
            return RepairOrderRepository::getInstance().getStatusStatistics();
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to get repair status statistics: %s", e.what());
            throw;
        }
    }

private:
    RepairService(const RepairService&) = delete;
    RepairService& operator=(const RepairService&) = delete;

public:
    static RepairService& getInstance() {
        static RepairService instance;
        return instance;
    }
};
