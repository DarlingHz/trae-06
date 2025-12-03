# 共享停车位预约与管理系统

一个使用C++17开发的高性能后端服务，提供停车位的共享、搜索和预约管理功能。

## 技术栈

- **C++17**: 核心开发语言
- **cpp-httplib**: HTTP服务器框架
- **SQLite**: 轻量级数据库
- **nlohmann/json**: JSON处理库
- **CMake**: 构建工具
- **OpenSSL**: 密码哈希

## 功能特性

### 用户管理
- 用户注册和登录
- Token认证会话管理
- 用户信息查询

### 停车位管理
- 发布停车位
- 更新停车位信息
- 搜索可用停车位
- 停车位停用/激活

### 预约管理
- 创建预约
- 取消预约
- 完成预约
- 预约冲突检测
- 按时间段和地点搜索

## 项目结构

```
.
├── CMakeLists.txt          # CMake构建配置
├── config.json.example     # 配置文件示例
├── README.md               # 项目说明文档
├── include/                # 头文件目录
│   └── parking/
│       ├── config.h        # 配置管理
│       ├── controllers.h   # HTTP控制器
│       ├── dao.h           # 数据访问层
│       ├── database.h      # 数据库管理
│       ├── models.h        # 数据模型
│       ├── services.h      # 业务服务层
│       └── utils.h         # 工具类
├── sql/                    # SQL脚本
│   └── init.sql            # 数据库初始化脚本
├── src/                    # 源文件目录
│   ├── controllers/        # HTTP控制器实现
│   ├── dao/                # 数据访问层实现
│   ├── models/             # 模型工具函数实现
│   ├── services/           # 业务服务层实现
│   └── main.cpp            # 主入口文件
├── tests/                  # 测试文件
│   └── test_config.json    # 测试配置
└── build/                  # 构建输出目录
```

## 构建和运行

### 环境要求
- C++17编译器
- CMake 3.16+
- OpenSSL开发库

### 构建步骤

1. 克隆项目：
```bash
git clone <repository-url>
cd parking-system
```

2. 创建配置文件：
```bash
cp config.json.example config.json
```

3. 构建：
```bash
mkdir -p build
cd build
cmake ..
make
```

4. 运行：
```bash
./parking-system
```

服务器将在 http://localhost:8080 启动

## API 接口

### 用户接口

#### 注册
```
POST /api/users/register
Content-Type: application/json

{
  "name": "张三",
  "email": "zhangsan@example.com",
  "password": "password123"
}
```

#### 登录
```
POST /api/users/login
Content-Type: application/json

{
  "email": "zhangsan@example.com",
  "password": "password123"
}
```

#### 获取当前用户信息
```
GET /api/users/me
X-Auth-Token: <your-token>
```

#### 登出
```
POST /api/users/logout
X-Auth-Token: <your-token>
```

### 停车位接口

#### 创建停车位
```
POST /api/parking-spots
X-Auth-Token: <your-token>
Content-Type: application/json

{
  "title": "小区地下停车位",
  "address": "北京市朝阳区XX小区",
  "price_per_hour": 10.0,
  "daily_available_start": "08:00",
  "daily_available_end": "22:00",
  "latitude": 39.9042,
  "longitude": 116.4074
}
```

#### 获取我的停车位
```
GET /api/parking-spots/my
X-Auth-Token: <your-token>
```

#### 更新停车位
```
PUT /api/parking-spots/:id
X-Auth-Token: <your-token>
Content-Type: application/json

{
  "title": "小区地下停车位(更新)",
  "address": "北京市朝阳区XX小区",
  "price_per_hour": 12.0,
  "daily_available_start": "09:00",
  "daily_available_end": "23:00"
}
```

#### 停用停车位
```
DELETE /api/parking-spots/:id
X-Auth-Token: <your-token>
```

#### 搜索停车位
```
GET /api/parking-spots/search?city=北京&start_time=1678291200&end_time=1678302000
```

### 预约接口

#### 创建预约
```
POST /api/reservations
X-Auth-Token: <your-token>
Content-Type: application/json

{
  "spot_id": 1,
  "start_time": 1678291200,
  "end_time": 1678302000,
  "vehicle_plate": "京A12345"
}
```

#### 获取我的预约
```
GET /api/reservations/my
X-Auth-Token: <your-token>
```

#### 获取我的车位预约
```
GET /api/reservations/for-my-spots
X-Auth-Token: <your-token>
```

#### 取消预约
```
POST /api/reservations/:id/cancel
X-Auth-Token: <your-token>
```

#### 完成预约
```
POST /api/reservations/:id/finish
X-Auth-Token: <your-token>
```

## 配置文件

```json
{
  "port": 8080,
  "db_path": "parking.db",
  "token_expiration_hours": 24,
  "min_reservation_duration_hours": 1,
  "max_reservation_duration_hours": 24
}
```

## 数据库设计

### Users表
- id (INTEGER PRIMARY KEY)
- name (TEXT NOT NULL)
- email (TEXT NOT NULL UNIQUE)
- password_hash (TEXT NOT NULL)
- status (TEXT NOT NULL)
- created_at (INTEGER NOT NULL)
- updated_at (INTEGER NOT NULL)

### ParkingSpots表
- id (INTEGER PRIMARY KEY)
- owner_user_id (INTEGER NOT NULL)
- title (TEXT NOT NULL)
- address (TEXT NOT NULL)
- latitude (DOUBLE)
- longitude (DOUBLE)
- price_per_hour (DOUBLE NOT NULL)
- daily_available_start (INTEGER NOT NULL)
- daily_available_end (INTEGER NOT NULL)
- status (TEXT NOT NULL)
- created_at (INTEGER NOT NULL)
- updated_at (INTEGER NOT NULL)

### Reservations表
- id (INTEGER PRIMARY KEY)
- spot_id (INTEGER NOT NULL)
- renter_user_id (INTEGER NOT NULL)
- owner_user_id (INTEGER NOT NULL)
- vehicle_plate (TEXT NOT NULL)
- start_time (INTEGER NOT NULL)
- end_time (INTEGER NOT NULL)
- total_price (DOUBLE NOT NULL)
- status (TEXT NOT NULL)
- created_at (INTEGER NOT NULL)
- updated_at (INTEGER NOT NULL)

### Sessions表
- token (TEXT PRIMARY KEY)
- user_id (INTEGER NOT NULL)
- expires_at (INTEGER NOT NULL)
- created_at (INTEGER NOT NULL)

## 测试

### 运行测试
```bash
cd build
make test
```

### 测试用例
- 用户注册登录测试
- CRUD测试
- 预约流程测试
- 并发冲突测试
- 错误处理测试

## 开发规范

1. **C++标准**: 使用C++17特性
2. **命名规范**: 
   - 类名: PascalCase
   - 函数名: camelCase
   - 变量名: camelCase
   - 常量名: UPPER_SNAKE_CASE
3. **代码风格**: 遵循Google C++风格指南
4. **错误处理**: 使用异常处理
5. **文档**: 重要函数和类添加Doxygen注释

## 许可证

MIT License
