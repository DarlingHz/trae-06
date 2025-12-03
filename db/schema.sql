-- 数据库初始化脚本
-- 线下活动报名与签到管理系统

-- 创建事件表
CREATE TABLE IF NOT EXISTS events (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    description TEXT,
    start_time INTEGER NOT NULL,
    end_time INTEGER NOT NULL,
    location TEXT NOT NULL,
    capacity INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'DRAFT',
    created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
    updated_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now'))
);

-- 创建用户表
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT NOT NULL UNIQUE,
    phone TEXT,
    created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now'))
);

-- 创建报名表
CREATE TABLE IF NOT EXISTS registrations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    event_id INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'REGISTERED',
    created_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
    updated_at INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (event_id) REFERENCES events(id) ON DELETE CASCADE,
    UNIQUE(user_id, event_id)
);

-- 创建签到日志表
CREATE TABLE IF NOT EXISTS checkin_logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    registration_id INTEGER NOT NULL,
    check_in_time INTEGER NOT NULL DEFAULT (strftime('%s', 'now')),
    channel TEXT NOT NULL DEFAULT 'MANUAL',
    FOREIGN KEY (registration_id) REFERENCES registrations(id) ON DELETE CASCADE
);

-- 创建索引以优化查询性能
-- 按活动ID和状态查询报名记录
CREATE INDEX IF NOT EXISTS idx_registrations_event_id_status ON registrations(event_id, status);

-- 按用户ID查询报名记录
CREATE INDEX IF NOT EXISTS idx_registrations_user_id ON registrations(user_id);

-- 按签到时间查询签到记录
CREATE INDEX IF NOT EXISTS idx_checkin_logs_time ON checkin_logs(check_in_time);

-- 按标题和描述模糊搜索事件
CREATE INDEX IF NOT EXISTS idx_events_title_description ON events(title, description);

-- 按时间范围查询事件
CREATE INDEX IF NOT EXISTS idx_events_time_range ON events(start_time, end_time);
