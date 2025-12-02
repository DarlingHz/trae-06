-- 创建数据库（如果需要）
-- CREATE DATABASE IF NOT EXISTS library;
-- USE library;

-- 用户表
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    nickname VARCHAR(50) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    role ENUM('reader', 'admin') DEFAULT 'reader',
    status ENUM('active', 'inactive') DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_users_username (username),
    INDEX idx_users_email (email),
    INDEX idx_users_role (role)
);

-- 图书表
CREATE TABLE IF NOT EXISTS books (
    id INT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(200) NOT NULL,
    author VARCHAR(100) NOT NULL,
    isbn VARCHAR(20) NOT NULL UNIQUE,
    description TEXT,
    total_copies INT NOT NULL DEFAULT 0,
    available_copies INT NOT NULL DEFAULT 0,
    borrowed_copies INT NOT NULL DEFAULT 0,
    status ENUM('active', 'inactive') DEFAULT 'active',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_books_title (title),
    INDEX idx_books_author (author),
    INDEX idx_books_isbn (isbn),
    INDEX idx_books_status (status),
    INDEX idx_books_available_copies (available_copies)
);

-- 分类表
CREATE TABLE IF NOT EXISTS categories (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) NOT NULL UNIQUE,
    description VARCHAR(200),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_categories_name (name)
);

-- 图书分类关联表
CREATE TABLE IF NOT EXISTS book_categories (
    id INT AUTO_INCREMENT PRIMARY KEY,
    book_id INT NOT NULL,
    category_id INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    FOREIGN KEY (category_id) REFERENCES categories(id) ON DELETE CASCADE,
    UNIQUE KEY uk_book_category (book_id, category_id),
    INDEX idx_book_categories_book_id (book_id),
    INDEX idx_book_categories_category_id (category_id)
);

-- 借阅记录表
CREATE TABLE IF NOT EXISTS borrow_records (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    book_id INT NOT NULL,
    borrow_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    due_date TIMESTAMP NOT NULL,
    return_date TIMESTAMP NULL DEFAULT NULL,
    status ENUM('borrowed', 'returned', 'overdue', 'overdue_returned') DEFAULT 'borrowed',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    INDEX idx_borrow_records_user_id (user_id),
    INDEX idx_borrow_records_book_id (book_id),
    INDEX idx_borrow_records_status (status),
    INDEX idx_borrow_records_due_date (due_date)
);

-- 预约记录表
CREATE TABLE IF NOT EXISTS reservation_records (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    book_id INT NOT NULL,
    reservation_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status ENUM('pending', 'confirmed', 'cancelled', 'expired') DEFAULT 'pending',
    confirmed_date TIMESTAMP NULL DEFAULT NULL,
    expire_date TIMESTAMP NULL DEFAULT NULL,
    queue_position INT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
    UNIQUE KEY uk_user_book_reservation (user_id, book_id, status) WHERE status = 'pending',
    INDEX idx_reservation_records_user_id (user_id),
    INDEX idx_reservation_records_book_id (book_id),
    INDEX idx_reservation_records_status (status),
    INDEX idx_reservation_records_queue_position (queue_position)
);

-- 插入初始数据
-- 插入管理员用户（密码：admin123，需要在代码中进行哈希处理）
INSERT INTO users (username, nickname, email, password_hash, role) 
VALUES ('admin', '管理员', 'admin@library.com', '$2a$12$LQv3c1yqBWVHxkdqHjJdCOYz6kA6dJZ6eZ6kA6dJZ6eZ6kA6dJZ6', 'admin') 
ON DUPLICATE KEY UPDATE username = username;

-- 插入示例分类
INSERT INTO categories (name, description) VALUES 
('文学', '包括小说、诗歌、散文等'),
('科技', '包括计算机、工程、自然科学等'),
('历史', '包括世界历史、中国历史等'),
('艺术', '包括绘画、音乐、戏剧等'),
('哲学', '包括伦理学、逻辑学、美学等') 
ON DUPLICATE KEY UPDATE name = name;

-- 插入示例图书
INSERT INTO books (title, author, isbn, description, total_copies, available_copies, borrowed_copies) VALUES 
('三体', '刘慈欣', '9787536692930', '一部震撼人心的科幻小说，讲述了地球文明与三体文明的博弈。', 5, 5, 0),
('活着', '余华', '9787506392558', '讲述了一个人和他命运之间的友情，这是最为感人的友情。', 3, 3, 0),
('百年孤独', '加西亚·马尔克斯', '9787544291170', '魔幻现实主义文学的代表作，描述了布恩迪亚家族七代人的传奇故事。', 2, 2, 0),
('人类简史', '尤瓦尔·赫拉利', '9787508647357', '从石器时代到21世纪，重新理解人类历史的发展进程。', 4, 4, 0),
('深度学习', 'Ian Goodfellow', '9787115461476', '全面介绍深度学习的理论基础、实践方法和应用领域。', 3, 3, 0) 
ON DUPLICATE KEY UPDATE isbn = isbn;

-- 为示例图书分配分类
INSERT INTO book_categories (book_id, category_id) VALUES 
(1, 2), -- 三体 -> 科技
(2, 1), -- 活着 -> 文学
(3, 1), -- 百年孤独 -> 文学
(4, 3), -- 人类简史 -> 历史
(5, 2)  -- 深度学习 -> 科技
ON DUPLICATE KEY UPDATE book_id = book_id, category_id = category_id;