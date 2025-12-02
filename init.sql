-- 创建数据库
CREATE DATABASE IF NOT EXISTS giftcard_db DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE giftcard_db;

-- 礼品卡模板表
CREATE TABLE IF NOT EXISTS giftcard_templates (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100) NOT NULL COMMENT '模板名称',
    type ENUM('amount', 'discount') NOT NULL COMMENT '模板类型：固定面额/折扣',
    face_value DECIMAL(10,2) NOT NULL COMMENT '面额或折扣百分比（折扣时1~100）',
    min_order_amount DECIMAL(10,2) DEFAULT 0.00 COMMENT '最低使用订单金额',
    total_stock INT UNSIGNED NOT NULL COMMENT '总库存',
    issued_count INT UNSIGNED DEFAULT 0 COMMENT '已发放数量',
    per_user_limit INT UNSIGNED DEFAULT 1 COMMENT '单个用户最多领取数量',
    valid_from DATETIME NOT NULL COMMENT '有效期开始时间',
    valid_to DATETIME NOT NULL COMMENT '有效期结束时间',
    status ENUM('active', 'closed') DEFAULT 'active' COMMENT '状态：活跃/已关闭',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_status (status),
    INDEX idx_valid_from (valid_from),
    INDEX idx_valid_to (valid_to)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='礼品卡模板表';

-- 礼品卡表
CREATE TABLE IF NOT EXISTS giftcards (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    card_no VARCHAR(32) NOT NULL UNIQUE COMMENT '卡号',
    user_id BIGINT UNSIGNED NOT NULL COMMENT '用户ID',
    template_id BIGINT UNSIGNED NOT NULL COMMENT '模板ID',
    balance DECIMAL(10,2) NOT NULL COMMENT '余额（固定面额卡）',
    discount_rate DECIMAL(5,2) DEFAULT 0.00 COMMENT '折扣率（折扣卡）',
    valid_from DATETIME NOT NULL COMMENT '有效期开始时间',
    valid_to DATETIME NOT NULL COMMENT '有效期结束时间',
    status ENUM('available', 'locked', 'used', 'expired', 'frozen') DEFAULT 'available' COMMENT '状态',
    version INT UNSIGNED DEFAULT 1 COMMENT '版本号（乐观锁）',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_user_id (user_id),
    INDEX idx_template_id (template_id),
    INDEX idx_status (status),
    INDEX idx_valid_to (valid_to),
    FOREIGN KEY (template_id) REFERENCES giftcard_templates(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='礼品卡表';

-- 礼品卡锁定记录表
CREATE TABLE IF NOT EXISTS giftcard_locks (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    card_id BIGINT UNSIGNED NOT NULL COMMENT '礼品卡ID',
    user_id BIGINT UNSIGNED NOT NULL COMMENT '用户ID',
    order_id BIGINT UNSIGNED NOT NULL COMMENT '订单ID',
    lock_amount DECIMAL(10,2) NOT NULL COMMENT '锁定金额（固定面额卡）',
    lock_ttl DATETIME NOT NULL COMMENT '锁定过期时间',
    status ENUM('active', 'consumed', 'released') DEFAULT 'active' COMMENT '状态：活跃/已消费/已释放',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_card_id (card_id),
    INDEX idx_user_id (user_id),
    INDEX idx_order_id (order_id),
    INDEX idx_status (status),
    INDEX idx_lock_ttl (lock_ttl),
    FOREIGN KEY (card_id) REFERENCES giftcards(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='礼品卡锁定记录表';

-- 礼品卡消费记录表
CREATE TABLE IF NOT EXISTS giftcard_consumptions (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    card_id BIGINT UNSIGNED NOT NULL COMMENT '礼品卡ID',
    user_id BIGINT UNSIGNED NOT NULL COMMENT '用户ID',
    order_id BIGINT UNSIGNED NOT NULL COMMENT '订单ID',
    consume_amount DECIMAL(10,2) NOT NULL COMMENT '消费金额（固定面额卡）',
    consume_time DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '消费时间',
    INDEX idx_card_id (card_id),
    INDEX idx_user_id (user_id),
    INDEX idx_order_id (order_id),
    INDEX idx_consume_time (consume_time),
    FOREIGN KEY (card_id) REFERENCES giftcards(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='礼品卡消费记录表';

-- 幂等键记录表
CREATE TABLE IF NOT EXISTS idempotency_keys (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,
    key_value VARCHAR(64) NOT NULL UNIQUE COMMENT '幂等键值',
    request_data TEXT COMMENT '请求数据（可选）',
    response_data TEXT COMMENT '响应数据（可选）',
    status ENUM('processing', 'completed') DEFAULT 'processing' COMMENT '状态',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_key_value (key_value)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='幂等键记录表';

-- 插入示例数据
INSERT INTO giftcard_templates (name, type, face_value, min_order_amount, total_stock, per_user_limit, valid_from, valid_to) 
VALUES 
('100元礼品卡', 'amount', 100.00, 0.00, 1000, 5, NOW(), DATE_ADD(NOW(), INTERVAL 1 YEAR)),
('8折优惠券', 'discount', 80.00, 100.00, 500, 2, NOW(), DATE_ADD(NOW(), INTERVAL 6 MONTH));
