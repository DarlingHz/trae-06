# 设备保修管理系统

一个基于C++的设备保修管理系统，提供完整的设备全生命周期管理，从注册、保修策略管理到维修服务。

## 功能特性

### 核心功能
- **用户管理**: 用户注册、登录、信息管理
- **设备管理**: 设备注册、保修状态查询、设备列表
- **保修策略管理**: 保修策略创建、查询、更新
- **维修网点管理**: 网点信息管理、按城市查询
- **维修订单管理**: 维修申请、状态跟踪、历史记录
- **保修提醒**: 保修到期自动提醒
- **统计分析**: 保修覆盖率、维修率、返修率统计

### 技术特性
- 基于C++17的高性能后端
- 单例模式管理核心组件
- MySQL数据库支持
- Redis缓存集成
- RESTful API接口
- 异步任务处理
- 日志系统
- 完整的异常处理

## 技术栈

- **语言**: C++17
- **数据库**: MySQL 8.0+
- **缓存**: Redis 6.0+
- **依赖库**: 
  - MySQL Connector/C++
  - hiredis (Redis客户端)
  - pthread (线程库)
- **构建工具**: CMake 3.10+

## 项目结构

```
project/
├── src/
│   ├── controllers/          # HTTP控制器和路由
│   ├── models/               # 数据模型
│   ├── repositories/         # 数据访问层
│   ├── services/             # 业务逻辑层
│   ├── utils/                # 工具类
│   └── main.cpp              # 主程序入口
├── test/                     # 测试文件
├── sql/                      # 数据库脚本
│   └── init.sql              # 初始化脚本
├── config.ini                # 配置文件
├── CMakeLists.txt            # CMake配置
└── README.md                 # 项目文档
```

## 安装和运行

### 环境要求
- macOS/Linux系统
- C++17编译器
- CMake 3.10+
- MySQL 8.0+
- Redis 6.0+

### 1. 安装依赖

```bash
# Ubuntu/Debian
sudo apt-get install g++ cmake libmysqlclient-dev libhiredis-dev

# macOS (Homebrew)
brew install cmake mysql hiredis
```

### 2. 创建数据库

```bash
mysql -u root -p
CREATE DATABASE device_warranty;
```

### 3. 导入数据库脚本

```bash
mysql -u root -p device_warranty < sql/init.sql
```

### 4. 配置项目

编辑 `config.ini` 文件，设置数据库连接信息和其他配置：

```ini
[Database]
host=localhost
port=3306
user=root
password=your_password
db_name=device_warranty
pool_size=10

[Redis]
enabled=true
host=localhost
port=6379
password=
db=0

[Server]
port=8000

[Logging]
level=INFO
log_file=logs/app.log
```

### 5. 编译项目

```bash
mkdir build
cd build
cmake ..
make -j4
```

### 6. 运行程序

```bash
./warranty_service
```

## API 接口

### 用户管理
- `POST /api/users` - 创建用户
- `GET /api/users/{id}` - 获取用户信息
- `GET /api/users?email={email}` - 按邮箱查询用户
- `PUT /api/users/{id}` - 更新用户信息
- `DELETE /api/users/{id}` - 删除用户

### 设备管理
- `POST /api/devices` - 创建设备
- `GET /api/devices/{id}` - 获取设备信息
- `GET /api/devices/user/{userId}` - 获取用户设备列表
- `PUT /api/devices/{id}` - 更新设备信息
- `DELETE /api/devices/{id}` - 删除设备
- `GET /api/devices/{id}/warranty` - 检查设备保修状态

### 保修策略管理
- `POST /api/warranty-policies` - 创建保修策略
- `GET /api/warranty-policies/{id}` - 获取保修策略信息
- `GET /api/warranty-policies` - 查询保修策略
- `PUT /api/warranty-policies/{id}` - 更新保修策略
- `DELETE /api/warranty-policies/{id}` - 删除保修策略

### 维修网点管理
- `POST /api/service-centers` - 创建维修网点
- `GET /api/service-centers/{id}` - 获取网点信息
- `GET /api/service-centers/city/{city}` - 按城市查询网点
- `PUT /api/service-centers/{id}` - 更新网点信息
- `DELETE /api/service-centers/{id}` - 删除网点

### 维修订单管理
- `POST /api/repair-orders` - 创建维修订单
- `GET /api/repair-orders/{id}` - 获取维修订单信息
- `GET /api/repair-orders` - 查询维修订单
- `PUT /api/repair-orders/{id}/status` - 更新维修状态
- `GET /api/repair-orders/{id}/history` - 获取维修状态历史
- `GET /api/repair-orders/stats` - 获取维修统计信息

## API 示例

### 创建用户
```bash
curl -X POST http://localhost:8000/api/users \
  -H "Content-Type: application/json" \
  -d '{
    "name": "张三",
    "email": "zhangsan@example.com",
    "phone": "13800138000"
  }'
```

### 创建设备
```bash
curl -X POST http://localhost:8000/api/devices \
  -H "Content-Type: application/json" \
  -d '{
    "userId": 1,
    "serialNumber": "A1234567890",
    "brand": "Apple",
    "model": "iPhone 14 Pro",
    "purchaseDate": "2023-01-15",
    "warrantyEndDate": "2024-01-15",
    "category": "Phone"
  }'
```

### 创建维修订单
```bash
curl -X POST http://localhost:8000/api/repair-orders \
  -H "Content-Type: application/json" \
  -d '{
    "userId": 1,
    "deviceId": 1,
    "serviceCenterId": 1,
    "issueDescription": "屏幕碎裂"
  }'
```

## 测试

运行单元测试：

```bash
cd build
./test_warranty_service
```

## 缓存策略

系统使用Redis缓存以下数据：
- 用户设备列表 (缓存5分钟)
- 设备保修状态 (缓存10分钟)
- 保修提醒 (缓存30分钟)
- 维修网点信息 (缓存1小时)

## 错误处理

系统使用HTTP状态码表示错误：
- 400 Bad Request: 参数错误
- 404 Not Found: 资源不存在
- 500 Internal Server Error: 服务器错误

## 开发规范

### 代码风格
- 使用C++17特性
- 遵循Google C++编码规范
- 使用智能指针管理内存
- 异常处理确保资源正确释放

### 数据库规范
- 使用预编译语句防止SQL注入
- 连接池管理数据库连接
- 事务处理确保数据一致性

## 许可证

MIT License

## 贡献

欢迎提交Issue和Pull Request！
