# 线下活动报名与签到管理系统

这是一个基于 C++ 开发的线下活动报名与签到管理后端服务，提供 RESTful API 接口。

## 技术栈

- **HTTP 框架**: cpp-httplib v0.10.6
- **JSON 库**: nlohmann/json v3.11.2
- **日志库**: spdlog v1.11.0
- **数据库**: SQLite3 + C API
- **构建系统**: CMake 3.15+

## 功能特性

### 活动管理
- 创建活动（标题、描述、时间、地点、容量、状态）
- 更新活动信息（支持部分字段修改）
- 获取活动列表（支持分页、搜索、过滤）
- 获取活动详情
- 获取活动统计信息

### 用户管理
- 创建用户（邮箱唯一）
- 获取用户详情
- 获取用户报名记录

### 报名管理
- 用户报名活动（支持邮箱或 user_id）
- 自动处理名额限制和等候名单
- 取消报名（自动提升等候名单）
- 获取活动报名列表

### 签到管理
- 用户签到（支持 registration_id、user_id 或 email）
- 记录签到渠道和时间
- 防重复签到

### 健康检查
- 数据库连接状态
- 服务版本信息

## 项目结构

```
.
├── config/                # 配置文件
│   └── config.json       # 应用配置
├── db/                   # 数据库相关
│   └── schema.sql        # 数据库初始化脚本
├── include/              # 头文件
│   ├── config/           # 配置模块
│   ├── controller/       # 控制器层
│   ├── model/            # 数据模型
│   ├── repository/       # 数据库访问层
│   └── service/          # 业务逻辑层
├── src/                  # 源文件
│   ├── config/           # 配置模块实现
│   ├── controller/       # 控制器实现
│   ├── model/            # 数据模型实现
│   ├── repository/       # 数据库访问层实现
│   ├── service/          # 业务逻辑层实现
│   └── main.cpp          # 程序入口
├── CMakeLists.txt        # CMake 配置
└── README.md            # 项目文档
```

## 安装依赖

### macOS (使用 Homebrew)

```bash
brew install cpp-httplib nlohmann-json spdlog sqlite3
```

### Ubuntu/Debian

```bash
sudo apt-get install libcpp-httplib-dev libnlohmann-json-dev libspdlog-dev libsqlite3-dev
```

## 构建项目

```bash
mkdir build && cd build
cmake ..
make -j4
```

构建完成后，可执行文件位于 `build/bin/event_signup_service`

## 数据库初始化

首次运行前需要初始化数据库：

```bash
mkdir -p data
sqlite3 data/event_signup.db < db/schema.sql
```

## 运行服务

```bash
# 方式1：直接运行
build/bin/event_signup_service

# 方式2：指定配置文件路径
build/bin/event_signup_service --config config/config.json
```

默认服务将在 `http://localhost:8080` 启动

## API 接口文档

### 活动相关

#### 创建活动
```bash
POST /events
Content-Type: application/json

{
  "title": "技术分享会",
  "description": "关于 C++ 并发编程的分享",
  "start_time": 1640995200,  # Unix时间戳
  "end_time": 1641009600,
  "location": "北京朝阳区",
  "capacity": 50,
  "status": "DRAFT"
}
```

#### 获取活动列表
```bash
GET /events?page=1&page_size=20&keyword=C++&status=PUBLISHED
```

#### 获取活动详情
```bash
GET /events/{event_id}
```

#### 获取活动统计
```bash
GET /events/{event_id}/stats
```

### 用户相关

#### 创建用户
```bash
POST /users
Content-Type: application/json

{
  "name": "张三",
  "email": "zhangsan@example.com",
  "phone": "13800138000"
}
```

### 报名相关

#### 用户报名活动
```bash
POST /events/{event_id}/register
Content-Type: application/json

{
  "email": "zhangsan@example.com"
}
```

#### 取消报名
```bash
POST /events/{event_id}/cancel
Content-Type: application/json

{
  "email": "zhangsan@example.com"
}
```

### 签到相关

#### 用户签到
```bash
POST /events/{event_id}/checkin
Content-Type: application/json

{
  "email": "zhangsan@example.com",
  "channel": "QR_CODE"
}
```

### 健康检查
```bash
GET /healthz
```

## 完整流程示例

### 1. 创建活动
```bash
curl -X POST http://localhost:8080/events \
  -H "Content-Type: application/json" \
  -d '{
    "title": "技术分享会",
    "description": "关于 C++ 并发编程的分享",
    "start_time": 1640995200,
    "end_time": 1641009600,
    "location": "北京朝阳区",
    "capacity": 50,
    "status": "PUBLISHED"
  }'
```

### 2. 创建用户
```bash
curl -X POST http://localhost:8080/users \
  -H "Content-Type: application/json" \
  -d '{
    "name": "张三",
    "email": "zhangsan@example.com",
    "phone": "13800138000"
  }'
```

### 3. 报名活动
```bash
curl -X POST http://localhost:8080/events/1/register \
  -H "Content-Type: application/json" \
  -d '{"email": "zhangsan@example.com"}'
```

### 4. 签到
```bash
curl -X POST http://localhost:8080/events/1/checkin \
  -H "Content-Type: application/json" \
  -d '{
    "email": "zhangsan@example.com",
    "channel": "QR_CODE"
  }'
```

### 5. 查看统计
```bash
curl http://localhost:8080/events/1/stats
```

## 并发控制

- **报名操作**: 使用数据库事务和行级锁保证不会超卖
- **取消报名**: 使用事务保证等候名单提升的原子性
- **签到操作**: 使用事务防止重复签到
- **所有写操作**: 都在事务中执行，确保数据一致性

## 索引设计

- `idx_registrations_event_id_status`: 优化活动报名列表查询
- `idx_registrations_user_id`: 优化用户报名记录查询
- `idx_checkin_logs_time`: 优化签到时间范围查询
- `idx_events_title_description`: 优化活动搜索
- `idx_events_time_range`: 优化活动时间范围查询

## 配置说明

配置文件位于 `config/config.json`:

```json
{
  "service": {
    "port": 8080,
    "host": "0.0.0.0",
    "log_level": "info"
  },
  "database": {
    "path": "event_signup.db"
  }
}
```

日志级别支持: trace, debug, info, warn, err, critical, off

## 开发说明

- 使用 C++17 标准
- 代码遵循 Google 风格指南
- 使用智能指针管理资源
- 所有错误都有适当的异常处理
- 关键操作都有日志记录

## 日志格式

```
[2023-10-05 14:30:00] [info] 创建活动成功: id=1, title="技术分享会"
[2023-10-05 14:31:00] [info] 用户报名成功: user_id=1, event_id=1, status=REGISTERED
[2023-10-05 14:32:00] [info] 用户签到成功: registration_id=1, channel=QR_CODE
```

## 常见问题

### 数据库连接失败
- 检查配置文件中的数据库路径是否正确
- 确保数据库文件所在目录有写入权限
- 确认数据库已通过 `schema.sql` 初始化

### 端口被占用
- 修改 `config.json` 中的端口配置
- 或者使用 `lsof -i :8080` 查看占用进程并杀死

### 编译失败
- 检查所有依赖库是否已正确安装
- 确认 CMake 版本不低于 3.15
- 尝试清理 build 目录重新构建

## 许可证

MIT License
