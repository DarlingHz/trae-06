-- 宠物医院预约与病例管理系统数据库表结构

-- 创建数据库（如果使用 MySQL 需要此步骤，SQLite 不需要）
-- CREATE DATABASE IF NOT EXISTS pet_hospital;
-- USE pet_hospital;

-- 科室表
CREATE TABLE IF NOT EXISTS departments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(50) NOT NULL UNIQUE COMMENT '科室名称',
    description TEXT COMMENT '科室描述',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email VARCHAR(100) NOT NULL UNIQUE COMMENT '邮箱',
    password_hash VARCHAR(255) NOT NULL COMMENT '密码哈希',
    name VARCHAR(50) NOT NULL COMMENT '姓名',
    phone VARCHAR(20) COMMENT '电话',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 宠物表
CREATE TABLE IF NOT EXISTS pets (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL COMMENT '主人ID',
    name VARCHAR(50) NOT NULL COMMENT '宠物名称',
    species VARCHAR(30) NOT NULL COMMENT '物种（如：狗、猫）',
    breed VARCHAR(50) COMMENT '品种',
    gender TINYINT COMMENT '性别（0: 未知, 1: 公, 2: 母）',
    birthday DATE COMMENT '生日',
    weight DECIMAL(5,2) COMMENT '体重（kg）',
    notes TEXT COMMENT '备注',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 医生表
CREATE TABLE IF NOT EXISTS doctors (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    department_id INTEGER NOT NULL COMMENT '科室ID',
    name VARCHAR(50) NOT NULL COMMENT '医生姓名',
    title VARCHAR(30) COMMENT '职称（如：主任医师、主治医师）',
    specialty TEXT COMMENT '擅长方向',
    phone VARCHAR(20) COMMENT '电话',
    email VARCHAR(100) COMMENT '邮箱',
    available_start TIME DEFAULT '08:00:00' COMMENT '每日可预约开始时间',
    available_end TIME DEFAULT '18:00:00' COMMENT '每日可预约结束时间',
    is_active TINYINT DEFAULT 1 COMMENT '是否可用（1: 可用, 0: 不可用）',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (department_id) REFERENCES departments(id) ON DELETE RESTRICT
);

-- 预约表
CREATE TABLE IF NOT EXISTS appointments (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL COMMENT '预约用户ID',
    pet_id INTEGER NOT NULL COMMENT '宠物ID',
    doctor_id INTEGER NOT NULL COMMENT '医生ID',
    start_time DATETIME NOT NULL COMMENT '预约开始时间',
    end_time DATETIME NOT NULL COMMENT '预约结束时间',
    reason TEXT COMMENT '就诊原因',
    status TINYINT DEFAULT 0 COMMENT '状态（0: 待就诊, 1: 已完成, 2: 已取消）',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (pet_id) REFERENCES pets(id) ON DELETE CASCADE,
    FOREIGN KEY (doctor_id) REFERENCES doctors(id) ON DELETE RESTRICT,
    -- 确保同一医生同一时间段不被重复预约
    CONSTRAINT unique_doctor_time UNIQUE (doctor_id, start_time, end_time),
    -- 确保同一宠物同一时间段不被重复预约
    CONSTRAINT unique_pet_time UNIQUE (pet_id, start_time, end_time)
);

-- 病例表
CREATE TABLE IF NOT EXISTS records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    appointment_id INTEGER NOT NULL COMMENT '预约ID',
    chief_complaint TEXT NOT NULL COMMENT '主诉',
    diagnosis TEXT NOT NULL COMMENT '诊断结论',
    treatment TEXT COMMENT '治疗方案',
    medication TEXT COMMENT '用药建议',
    notes TEXT COMMENT '医生备注',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (appointment_id) REFERENCES appointments(id) ON DELETE CASCADE
);

-- Token 表（用于用户认证）
CREATE TABLE IF NOT EXISTS tokens (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL COMMENT '用户ID',
    token VARCHAR(255) NOT NULL UNIQUE COMMENT 'Token',
    expires_at DATETIME NOT NULL COMMENT '过期时间',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 插入初始科室数据
INSERT OR IGNORE INTO departments (name, description) VALUES
('内科', '治疗宠物内科疾病，如消化系统、呼吸系统、心血管系统等疾病'),
('外科', '治疗宠物外科疾病，如骨折、创伤、肿瘤切除、绝育手术等'),
('皮肤科', '治疗宠物皮肤疾病，如皮肤病、过敏、寄生虫感染等'),
('眼科', '治疗宠物眼部疾病，如结膜炎、白内障、青光眼等'),
('耳鼻喉科', '治疗宠物耳鼻喉疾病，如中耳炎、鼻炎、喉炎等'),
('牙科', '治疗宠物口腔疾病，如牙结石、牙周炎、牙齿拔除等'),
('急诊', '处理宠物紧急情况，如外伤、中毒、突发疾病等');

-- 插入初始医生数据
INSERT OR IGNORE INTO doctors (department_id, name, title, specialty, available_start, available_end) VALUES
(1, '张三', '主任医师', '擅长宠物消化系统疾病和内分泌疾病的诊断与治疗', '08:00:00', '17:00:00'),
(1, '李四', '主治医师', '擅长宠物呼吸系统疾病和心血管疾病的诊断与治疗', '09:00:00', '18:00:00'),
(2, '王五', '主任医师', '擅长宠物骨科手术和肿瘤切除手术', '08:00:00', '16:00:00'),
(2, '赵六', '主治医师', '擅长宠物外伤处理和绝育手术', '09:00:00', '17:00:00'),
(3, '孙七', '主任医师', '擅长宠物皮肤病和过敏反应的诊断与治疗', '08:00:00', '17:00:00'),
(7, '周八', '主治医师', '擅长宠物急诊处理和危重病监护', '00:00:00', '23:59:59');
