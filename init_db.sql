-- 城市失物招领系统数据库初始化脚本

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    phone TEXT,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL DEFAULT 'normal', -- normal, admin, staff
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 丢失物品表
CREATE TABLE IF NOT EXISTS lost_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    owner_user_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    category TEXT NOT NULL,
    lost_time DATETIME NOT NULL,
    lost_location TEXT NOT NULL,
    status TEXT NOT NULL DEFAULT 'open', -- open, matched, closed
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (owner_user_id) REFERENCES users(id)
);

-- 捡到物品表
CREATE TABLE IF NOT EXISTS found_items (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    finder_user_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    description TEXT,
    category TEXT NOT NULL,
    found_time DATETIME NOT NULL,
    found_location TEXT NOT NULL,
    keep_place TEXT NOT NULL,
    status TEXT NOT NULL DEFAULT 'open', -- open, matched, closed
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (finder_user_id) REFERENCES users(id)
);

-- 认领记录表
CREATE TABLE IF NOT EXISTS claims (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    lost_item_id INTEGER NOT NULL,
    found_item_id INTEGER NOT NULL,
    claimant_user_id INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'pending', -- pending, approved, rejected, cancelled
    evidence_text TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (lost_item_id) REFERENCES lost_items(id),
    FOREIGN KEY (found_item_id) REFERENCES found_items(id),
    FOREIGN KEY (claimant_user_id) REFERENCES users(id),
    UNIQUE(lost_item_id, found_item_id, status) -- 同一组合只允许一条approved
);

-- 通知表
CREATE TABLE IF NOT EXISTS notifications (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    message TEXT NOT NULL,
    type TEXT NOT NULL, -- claim_created, claim_approved, claim_rejected, etc.
    is_read INTEGER NOT NULL DEFAULT 0, -- 0: unread, 1: read
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id)
);

-- 索引优化
CREATE INDEX IF NOT EXISTS idx_lost_items_status ON lost_items(status);
CREATE INDEX IF NOT EXISTS idx_lost_items_category ON lost_items(category);
CREATE INDEX IF NOT EXISTS idx_lost_items_owner ON lost_items(owner_user_id);
CREATE INDEX IF NOT EXISTS idx_found_items_status ON found_items(status);
CREATE INDEX IF NOT EXISTS idx_found_items_category ON found_items(category);
CREATE INDEX IF NOT EXISTS idx_found_items_finder ON found_items(finder_user_id);
CREATE INDEX IF NOT EXISTS idx_claims_status ON claims(status);
CREATE INDEX IF NOT EXISTS idx_claims_lost_found ON claims(lost_item_id, found_item_id);
CREATE INDEX IF NOT EXISTS idx_notifications_user_read ON notifications(user_id, is_read);

-- 插入默认管理员用户
INSERT OR IGNORE INTO users (name, email, password_hash, role) 
VALUES ('Admin User', 'admin@example.com', 'admin123', 'admin');