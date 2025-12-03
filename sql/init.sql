-- Device Warranty Management System Database Schema

-- 关闭外键检查以便初始化
SET FOREIGN_KEY_CHECKS = 0;

-- 1. 用户表
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 2. 设备表
CREATE TABLE IF NOT EXISTS devices (
    id INT AUTO_INCREMENT PRIMARY KEY,
    owner_user_id INT NOT NULL,
    serial_number VARCHAR(100) NOT NULL UNIQUE,
    category ENUM('Phone', 'Laptop', 'Tablet', 'Other') NOT NULL DEFAULT 'Other',
    brand VARCHAR(255) NOT NULL,
    model VARCHAR(255) NOT NULL,
    purchase_date DATE NOT NULL,
    warranty_expire_at DATE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (owner_user_id) REFERENCES users(id) ON DELETE CASCADE,
    INDEX idx_owner_user_id (owner_user_id),
    INDEX idx_serial_number (serial_number),
    INDEX idx_warranty_expire_at (warranty_expire_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 3. 保修策略表
CREATE TABLE IF NOT EXISTS warranty_policies (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id INT NOT NULL,
    policy_type ENUM('Manufacturer', 'Extended', 'Accidental') NOT NULL,
    coverage_details TEXT,
    service_provider VARCHAR(255) NOT NULL,
    start_date DATE NOT NULL,
    expire_date DATE NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE CASCADE,
    INDEX idx_device_id (device_id),
    INDEX idx_expire_date (expire_date)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 4. 维修网点表
CREATE TABLE IF NOT EXISTS service_centers (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(255) NOT NULL,
    city VARCHAR(255) NOT NULL,
    address VARCHAR(500) NOT NULL,
    phone VARCHAR(20) NOT NULL,
    email VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_city (city)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 5. 维修订单表
CREATE TABLE IF NOT EXISTS repair_orders (
    id INT AUTO_INCREMENT PRIMARY KEY,
    device_id INT NOT NULL,
    user_id INT NOT NULL,
    service_center_id INT NOT NULL,
    issue_description TEXT NOT NULL,
    status ENUM('PendingReview', 'Accepted', 'PartsOrdered', 'Repaired', 'Rejected') NOT NULL DEFAULT 'PendingReview',
    estimated_cost DECIMAL(10, 2),
    actual_cost DECIMAL(10, 2),
    assigned_to VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (device_id) REFERENCES devices(id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (service_center_id) REFERENCES service_centers(id) ON DELETE CASCADE,
    INDEX idx_device_id (device_id),
    INDEX idx_user_id (user_id),
    INDEX idx_service_center_id (service_center_id),
    INDEX idx_status (status),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 6. 维修状态历史表
CREATE TABLE IF NOT EXISTS repair_status_history (
    id INT AUTO_INCREMENT PRIMARY KEY,
    repair_order_id INT NOT NULL,
    status ENUM('PendingReview', 'Accepted', 'PartsOrdered', 'Repaired', 'Rejected') NOT NULL,
    status_reason TEXT,
    changed_by VARCHAR(255) NOT NULL,
    changed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (repair_order_id) REFERENCES repair_orders(id) ON DELETE CASCADE,
    INDEX idx_repair_order_id (repair_order_id),
    INDEX idx_changed_at (changed_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 开启外键检查
SET FOREIGN_KEY_CHECKS = 1;

-- 插入初始数据
-- 维修网点示例数据
INSERT IGNORE INTO service_centers (name, city, address, phone, email) VALUES
('Apple Store', 'Beijing', '101 Wangfujing Street, Dongcheng District', '400-666-8800', 'beijing@apple.com'),
('Apple Store', 'Shanghai', '100 Nanjing East Road, Huangpu District', '400-666-8800', 'shanghai@apple.com'),
('Samsung Service Center', 'Guangzhou', '55 Tianhe Road, Tianhe District', '400-810-5858', 'guangzhou@samsung.com'),
('Huawei Customer Service Center', 'Shenzhen', '100 Binhai Boulevard, Nanshan District', '950800', 'shenzhen@huawei.com'),
('Xiaomi Service Center', 'Chengdu', '150 Chunxi Road, Jinjiang District', '400-100-5678', 'chengdu@xiaomi.com');

-- 用户示例数据
INSERT IGNORE INTO users (name, email) VALUES
('张三', 'zhangsan@example.com'),
('李四', 'lisi@example.com'),
('王五', 'wangwu@example.com'),
('赵六', 'zhaoliu@example.com'),
('钱七', 'qianqi@example.com');

-- 设备示例数据
INSERT IGNORE INTO devices (owner_user_id, serial_number, category, brand, model, purchase_date, warranty_expire_at) VALUES
(1, 'A1234567890', 'Phone', 'Apple', 'iPhone 14 Pro', '2023-01-15', '2024-01-15'),
(1, 'M2345678901', 'Laptop', 'Apple', 'MacBook Pro 16"', '2022-09-10', '2024-09-10'),
(2, 'G3456789012', 'Phone', 'Google', 'Pixel 7', '2023-03-20', '2025-03-20'),
(2, 'S4567890123', 'Tablet', 'Samsung', 'Galaxy Tab S9', '2023-02-01', '2024-02-01'),
(3, 'H5678901234', 'Phone', 'Huawei', 'Mate 60 Pro', '2023-10-01', '2025-10-01'),
(4, 'X6789012345', 'Phone', 'Xiaomi', '13 Ultra', '2023-04-15', '2024-04-15'),
(5, 'A7890123456', 'Phone', 'Apple', 'iPhone 13', '2022-06-10', '2023-06-10');

-- 保修策略示例数据
INSERT IGNORE INTO warranty_policies (device_id, policy_type, coverage_details, service_provider, start_date, expire_date) VALUES
(1, 'Manufacturer', 'Basic manufacturer warranty - covers defects in materials and workmanship for 1 year', 'Apple Inc.', '2023-01-15', '2024-01-15'),
(2, 'Manufacturer', 'Basic manufacturer warranty - covers defects in materials and workmanship for 1 year', 'Apple Inc.', '2022-09-10', '2023-09-10'),
(2, 'Extended', 'Extended AppleCare+ - covers accidental damage and extends warranty to 3 years', 'Apple Inc.', '2022-09-10', '2025-09-10'),
(3, 'Manufacturer', 'Basic manufacturer warranty - covers defects in materials and workmanship for 2 years', 'Google LLC', '2023-03-20', '2025-03-20'),
(5, 'Accidental', 'Huawei Premium Care - covers accidental damage and extends warranty to 2 years', 'Huawei Technologies Co., Ltd.', '2023-10-01', '2025-10-01'),
(7, 'Manufacturer', 'Basic manufacturer warranty - covers defects in materials and workmanship for 1 year', 'Apple Inc.', '2022-06-10', '2023-06-10');

-- 维修订单示例数据
INSERT IGNORE INTO repair_orders (device_id, user_id, service_center_id, issue_description, status, estimated_cost, actual_cost, assigned_to) VALUES
(1, 1, 1, '屏幕碎裂，触摸不灵敏', 'Accepted', 2000.00, NULL, '李技师'),
(7, 5, 1, '电池续航严重下降，需要更换电池', 'PendingReview', 599.00, NULL, NULL),
(4, 2, 4, '屏幕出现花屏，可能是排线问题', 'PartsOrdered', 800.00, NULL, '王工程师'),
(6, 4, 5, '无法开机，充电无反应', 'Rejected', NULL, NULL, '张维修员'),
(3, 2, 2, '摄像头拍照模糊，需要清洁或更换', 'Repaired', 300.00, 350.00, '赵技师');

-- 维修状态历史示例数据
INSERT IGNORE INTO repair_status_history (repair_order_id, status, status_reason, changed_by) VALUES
(1, 'PendingReview', '用户提交维修申请，等待审核', '系统自动'),
(1, 'Accepted', '维修单审核通过，等待分配', '审核员'),
(1, 'Accepted', '已分配给李技师', '李技师'),
(2, 'PendingReview', '用户提交维修申请，等待审核', '系统自动'),
(3, 'PendingReview', '用户提交维修申请，等待审核', '系统自动'),
(3, 'Accepted', '维修单审核通过，等待分配', '审核员'),
(3, 'PartsOrdered', '已订购所需维修零件，等待到货', '王工程师'),
(4, 'PendingReview', '用户提交维修申请，等待审核', '系统自动'),
(4, 'Rejected', '设备已过保修期，无法提供免费维修', '张维修员'),
(5, 'PendingReview', '用户提交维修申请，等待审核', '系统自动'),
(5, 'Accepted', '维修单审核通过，等待分配', '审核员'),
(5, 'Repaired', '设备维修完成，用户已取件', '赵技师');

-- 创建存储过程示例
DELIMITER //

-- 存储过程：获取即将过期的保修
CREATE PROCEDURE GetWarrantyUpcoming(IN days_before INT)
BEGIN
    SELECT 
        d.id AS device_id,
        d.serial_number,
        d.brand,
        d.model,
        d.warranty_expire_at,
        u.name AS owner_name,
        u.email AS owner_email
    FROM devices d
    JOIN users u ON d.owner_user_id = u.id
    WHERE d.warranty_expire_at BETWEEN CURRENT_DATE AND DATE_ADD(CURRENT_DATE, INTERVAL days_before DAY)
    ORDER BY d.warranty_expire_at ASC;
END //

-- 存储过程：获取维修状态统计
CREATE PROCEDURE GetRepairStatusStatistics()
BEGIN
    SELECT 
        status,
        COUNT(*) AS count
    FROM repair_orders
    GROUP BY status
    ORDER BY status;
END //

DELIMITER ;

-- 创建视图示例
-- 视图：用户设备保修信息
CREATE VIEW UserDeviceWarranty AS
SELECT 
    u.id AS user_id,
    u.name AS user_name,
    u.email AS user_email,
    d.id AS device_id,
    d.serial_number,
    d.brand,
    d.model,
    d.warranty_expire_at AS device_warranty_expire,
    d.purchase_date,
    d.category,
    wp.id AS warranty_policy_id,
    wp.policy_type,
    wp.service_provider,
    wp.expire_date AS policy_expire_date
FROM users u
LEFT JOIN devices d ON u.id = d.owner_user_id
LEFT JOIN warranty_policies wp ON d.id = wp.device_id
ORDER BY u.id, d.id, wp.id;

-- 视图：维修订单详情
CREATE VIEW RepairOrderDetails AS
SELECT 
    ro.id AS order_id,
    ro.issue_description,
    ro.status,
    ro.estimated_cost,
    ro.actual_cost,
    ro.created_at,
    ro.updated_at,
    u.name AS user_name,
    u.email AS user_email,
    d.brand AS device_brand,
    d.model AS device_model,
    d.serial_number AS device_serial,
    sc.name AS service_center_name,
    sc.city AS service_center_city,
    sc.address AS service_center_address
FROM repair_orders ro
JOIN users u ON ro.user_id = u.id
JOIN devices d ON ro.device_id = d.id
JOIN service_centers sc ON ro.service_center_id = sc.id
ORDER BY ro.created_at DESC;

-- 创建索引以提高查询性能
CREATE INDEX idx_device_owner_category ON devices(owner_user_id, category);
CREATE INDEX idx_warranty_policy_device_type ON warranty_policies(device_id, policy_type);
CREATE INDEX idx_repair_order_date_status ON repair_orders(created_at, status);
CREATE INDEX idx_repair_order_service_center_status ON repair_orders(service_center_id, status);
CREATE INDEX idx_repair_history_order_status ON repair_status_history(repair_order_id, status);

-- 创建事件调度器（需要开启事件调度器）
-- 事件：每天清理超过30天的已完成维修订单状态历史
DELIMITER //

CREATE EVENT IF NOT EXISTS CleanupRepairHistory
ON SCHEDULE EVERY 1 DAY
STARTS CURRENT_DATE + INTERVAL 1 DAY
DO
BEGIN
    DELETE FROM repair_status_history
    WHERE changed_at < DATE_SUB(CURRENT_DATE, INTERVAL 30 DAY)
    AND repair_order_id IN (
        SELECT id FROM repair_orders WHERE status = 'Repaired'
    );
END //

DELIMITER ;

-- 开启事件调度器（如果未开启）
SET GLOBAL event_scheduler = ON;

-- 创建备份表结构的存储过程
DELIMITER //

CREATE PROCEDURE BackupTables()
BEGIN
    -- 备份用户表
    CREATE TABLE IF NOT EXISTS users_backup LIKE users;
    REPLACE INTO users_backup SELECT * FROM users;
    
    -- 备份设备表
    CREATE TABLE IF NOT EXISTS devices_backup LIKE devices;
    REPLACE INTO devices_backup SELECT * FROM devices;
    
    -- 备份保修策略表
    CREATE TABLE IF NOT EXISTS warranty_policies_backup LIKE warranty_policies;
    REPLACE INTO warranty_policies_backup SELECT * FROM warranty_policies;
    
    -- 备份维修订单表
    CREATE TABLE IF NOT EXISTS repair_orders_backup LIKE repair_orders;
    REPLACE INTO repair_orders_backup SELECT * FROM repair_orders;
    
    -- 备份维修状态历史表
    CREATE TABLE IF NOT EXISTS repair_status_history_backup LIKE repair_status_history;
    REPLACE INTO repair_status_history_backup SELECT * FROM repair_status_history;
    
    -- 备份维修网点表
    CREATE TABLE IF NOT EXISTS service_centers_backup LIKE service_centers;
    REPLACE INTO service_centers_backup SELECT * FROM service_centers;
END //

DELIMITER ;

-- 创建用户权限（根据实际需求调整）
-- 创建只读用户
CREATE USER IF NOT EXISTS 'warranty_read'@'localhost' IDENTIFIED BY 'read_password';
GRANT SELECT ON warranty_db.* TO 'warranty_read'@'localhost';

-- 创建读写用户
CREATE USER IF NOT EXISTS 'warranty_write'@'localhost' IDENTIFIED BY 'write_password';
GRANT SELECT, INSERT, UPDATE, DELETE ON warranty_db.* TO 'warranty_write'@'localhost';

-- 创建管理员用户
CREATE USER IF NOT EXISTS 'warranty_admin'@'localhost' IDENTIFIED BY 'admin_password';
GRANT ALL PRIVILEGES ON warranty_db.* TO 'warranty_admin'@'localhost';

FLUSH PRIVILEGES;

-- 完成
SELECT 'Database schema initialization completed successfully!' AS message;
