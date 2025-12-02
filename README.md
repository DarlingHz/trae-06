# 健身房课程预约与会员训练记录系统

一个基于 C++17 和 oatpp 框架的后端服务，提供健身房课程预约、会员管理、教练管理、训练记录等功能。

## 技术栈

- **C++17**: 编程语言
- **oatpp**: HTTP 服务器框架和 ORM
- **SQLite**: 持久化存储
- **nlohmann/json**: JSON 序列化/反序列化
- **CMake**: 构建系统

## 项目结构

```
.
├── CMakeLists.txt          # CMake 配置文件
├── config.json             # 服务器配置文件
├── README.md               # 项目说明文档
└── src/
    ├── main.cpp            # 程序入口
    ├── controller/         # HTTP 控制器层
    ├── service/            # 业务服务层
    ├── data/               # 数据访问层
    ├── cache/              # 缓存层
    ├── util/               # 工具类
    └── dto/                # 数据传输对象
```

## 构建与运行

### 1. 安装依赖

#### macOS

```bash
# 安装 Homebrew
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# 安装 CMake
brew install cmake

# 安装 oatpp
brew install oatpp

# 安装 nlohmann/json
brew install nlohmann-json
```

### 2. 构建项目

```bash
# 创建构建目录
mkdir build
cd build

# 生成 Makefile
cmake ..

# 编译项目
make -j4
```

### 3. 运行服务器

```bash
# 进入构建目录
cd build

# 运行服务器
./gym_booking_system
```

服务器将在 `http://127.0.0.1:8080` 启动。

## API 接口示例

### 1. 创建会员

```bash
curl -X POST http://127.0.0.1:8080/api/members \
  -H "Content-Type: application/json" \
  -d '{"name":"张三","phone":"13800138000","level":"normal"}'
```

### 2. 创建教练

```bash
curl -X POST http://127.0.0.1:8080/api/coaches \
  -H "Content-Type: application/json" \
  -d '{"name":"李四","speciality":"力量训练"}'
```

### 3. 创建课程模板

```bash
curl -X POST http://127.0.0.1:8080/api/class_templates \
  -H "Content-Type: application/json" \
  -d '{"title":"燃脂团课","level_required":"normal","capacity":20,"duration_minutes":60,"coach_id":1}'
```

### 4. 发布课节

```bash
curl -X POST http://127.0.0.1:8080/api/class_sessions \
  -H "Content-Type: application/json" \
  -d '{"template_id":1,"start_time":"2024-01-15T19:00:00Z"}'
```

### 5. 预约课程

```bash
curl -X POST http://127.0.0.1:8080/api/bookings \
  -H "Content-Type: application/json" \
  -d '{"member_id":1,"session_id":1}'
```

### 6. 签到上课

```bash
curl -X POST http://127.0.0.1:8080/api/bookings/1/attend
```

### 7. 查询会员统计

```bash
curl http://127.0.0.1:8080/api/stats/member/1
```

## 核心功能

### 会员管理
- 创建会员
- 查询单个会员
- 按手机号查询会员

### 教练管理
- 创建教练
- 查询所有教练
- 查询单个教练

### 课程管理
- 创建课程模板
- 查询课程模板（支持筛选）
- 修改课程模板
- 发布课节
- 查询课节（支持时间范围和条件筛选）

### 预约与签到
- 预约课程
- 取消预约
- 签到上课
- 查询会员预约记录

### 训练记录
- 创建自助训练记录
- 查询会员训练记录

### 统计功能
- 会员训练统计（最近30天）
- 教练排课统计（未来7天）

## 性能优化

### 缓存机制
- 为课节查询接口实现内存缓存
- 缓存时间窗口：30秒
- 当有新课节创建或修改时，自动清空缓存

### 并发处理
- 使用 oatpp 自带的线程池处理并发请求
- 数据库连接池管理，确保线程安全

## 错误处理与日志

- 所有数据库操作进行错误捕获，返回合适的 HTTP 状态码
- 业务错误返回 4xx 状态码，并提供详细错误信息
- 关键操作记录结构化日志，包含时间、操作类型、影响对象等
