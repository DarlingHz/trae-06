-- 招聘管理系统数据库表结构
-- 公司表
CREATE TABLE IF NOT EXISTS company (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    industry TEXT,
    location TEXT,
    description TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 职位表
CREATE TABLE IF NOT EXISTS job (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    company_id INTEGER NOT NULL,
    title TEXT NOT NULL,
    location TEXT,
    salary_range TEXT,
    description TEXT,
    required_skills TEXT,
    is_open BOOLEAN DEFAULT TRUE,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (company_id) REFERENCES company(id) ON DELETE CASCADE
);

-- 候选人表
CREATE TABLE IF NOT EXISTS candidate (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    contact TEXT NOT NULL,
    resume TEXT,
    skills TEXT,
    years_of_experience INTEGER,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- 投递表
CREATE TABLE IF NOT EXISTS application (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    job_id INTEGER NOT NULL,
    candidate_id INTEGER NOT NULL,
    status TEXT NOT NULL DEFAULT 'applied',
    applied_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (job_id) REFERENCES job(id) ON DELETE CASCADE,
    FOREIGN KEY (candidate_id) REFERENCES candidate(id) ON DELETE CASCADE,
    UNIQUE(job_id, candidate_id)
);

-- 投递状态变更历史表
CREATE TABLE IF NOT EXISTS application_status_history (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    application_id INTEGER NOT NULL,
    from_status TEXT,
    to_status TEXT NOT NULL,
    changed_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (application_id) REFERENCES application(id) ON DELETE CASCADE
);

-- 面试表
CREATE TABLE IF NOT EXISTS interview (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    application_id INTEGER NOT NULL,
    scheduled_time DATETIME NOT NULL,
    interviewer_name TEXT NOT NULL,
    mode TEXT NOT NULL DEFAULT 'online',
    location_or_link TEXT,
    note TEXT,
    status TEXT NOT NULL DEFAULT 'scheduled',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (application_id) REFERENCES application(id) ON DELETE CASCADE
);

-- 评价表
CREATE TABLE IF NOT EXISTS evaluation (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    application_id INTEGER NOT NULL,
    interview_id INTEGER,
    score INTEGER NOT NULL CHECK(score >= 1 AND score <= 5),
    comment TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    evaluator TEXT NOT NULL,
    FOREIGN KEY (application_id) REFERENCES application(id) ON DELETE CASCADE,
    FOREIGN KEY (interview_id) REFERENCES interview(id) ON DELETE CASCADE
);

-- 创建索引以优化查询性能
-- 公司表索引
CREATE INDEX IF NOT EXISTS idx_company_industry ON company(industry);
CREATE INDEX IF NOT EXISTS idx_company_location ON company(location);

-- 职位表索引
CREATE INDEX IF NOT EXISTS idx_job_company_id ON job(company_id);
CREATE INDEX IF NOT EXISTS idx_job_location ON job(location);
CREATE INDEX IF NOT EXISTS idx_job_is_open ON job(is_open);
CREATE INDEX IF NOT EXISTS idx_job_created_at ON job(created_at);

-- 候选人表索引
CREATE INDEX IF NOT EXISTS idx_candidate_skills ON candidate(skills);
CREATE INDEX IF NOT EXISTS idx_candidate_years_of_experience ON candidate(years_of_experience);

-- 投递表索引
CREATE INDEX IF NOT EXISTS idx_application_job_id ON application(job_id);
CREATE INDEX IF NOT EXISTS idx_application_candidate_id ON application(candidate_id);
CREATE INDEX IF NOT EXISTS idx_application_status ON application(status);
CREATE INDEX IF NOT EXISTS idx_application_applied_at ON application(applied_at);

-- 投递状态变更历史表索引
CREATE INDEX IF NOT EXISTS idx_application_status_history_application_id ON application_status_history(application_id);
CREATE INDEX IF NOT EXISTS idx_application_status_history_changed_at ON application_status_history(changed_at);

-- 面试表索引
CREATE INDEX IF NOT EXISTS idx_interview_application_id ON interview(application_id);
CREATE INDEX IF NOT EXISTS idx_interview_scheduled_time ON interview(scheduled_time);
CREATE INDEX IF NOT EXISTS idx_interview_status ON interview(status);

-- 评价表索引
CREATE INDEX IF NOT EXISTS idx_evaluation_application_id ON evaluation(application_id);
CREATE INDEX IF NOT EXISTS idx_evaluation_interview_id ON evaluation(interview_id);
CREATE INDEX IF NOT EXISTS idx_evaluation_score ON evaluation(score);

-- 创建视图以简化常用查询
-- 职位详情视图（包含公司信息）
CREATE VIEW IF NOT EXISTS job_details AS
SELECT j.*, c.name AS company_name, c.industry AS company_industry, c.location AS company_location
FROM job j
JOIN company c ON j.company_id = c.id;

-- 投递详情视图（包含职位和候选人信息）
CREATE VIEW IF NOT EXISTS application_details AS
SELECT a.*, j.title AS job_title, j.company_id, c.name AS candidate_name, c.contact AS candidate_contact
FROM application a
JOIN job j ON a.job_id = j.id
JOIN candidate c ON a.candidate_id = c.id;

-- 面试详情视图（包含投递和候选人信息）
CREATE VIEW IF NOT EXISTS interview_details AS
SELECT i.*, a.job_id, a.candidate_id, c.name AS candidate_name, c.contact AS candidate_contact
FROM interview i
JOIN application a ON i.application_id = a.id
JOIN candidate c ON a.candidate_id = c.id;