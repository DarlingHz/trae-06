# 多用户网页书签与阅读清单管理后端服务

一个基于 C++17 的现代化后端服务，提供 HTTP+JSON API 接口，用于管理网页书签和阅读清单。支持多用户、标签系统、文件夹分类、统计分析等功能。

## 技术栈

- **C++17** - 现代化语言特性
- **CMake** - 构建系统
- **SQLite3** - 轻量级关系型数据库
- **cpp-httplib** - HTTP 服务器库
- **nlohmann_json** - JSON 处理库
- **OpenSSL** - 加密和哈希功能
- **GTest** - 单元测试框架
- **spdlog** - 日志库

## 功能特性

### 1. 用户管理
- 用户注册和登录
- JWT 身份验证
- 安全的密码哈希（PBKDF2 + SHA256）
- 用户信息管理

### 2. 书签管理
- 创建、读取、更新、删除书签
- URL 和标题验证
- 标签系统支持
- 文件夹分类
- 收藏功能
- 阅读状态跟踪（已读/未读）

### 3. 高级查询
- 分页查询
- 关键词搜索
- 标签过滤
- 文件夹过滤
- 阅读状态过滤
- 收藏过滤
- 多种排序方式（创建时间、访问时间等）

### 4. 批量操作
- 批量标记为已读/未读
- 批量移动到文件夹
- 批量删除

### 5. 统计分析
- 用户书签统计（总数、已读/未读、收藏数）
- 每日统计图表数据
- 热门域名统计
- 标签和文件夹统计

### 6. 性能优化
- 数据库连接池
- LRU 缓存机制
- 全文搜索优化
- 索引优化

### 7. 安全性
- JWT 令牌认证
- 密码哈希存储
- SQL 注入防护
- 输入验证和 sanitization

## 项目结构

```
project-root/
├── include/                    # 头文件目录
│   ├── auth/                  # 认证相关
│   │   └── JWT.h             # JWT 令牌处理
│   ├── cache/                 # 缓存相关
│   │   └── LRUCache.h        # LRU 缓存实现
│   ├── config/                # 配置相关
│   │   └── Config.h          # 配置结构体和加载
│   ├── models/                # 数据模型
│   │   ├── User.h            # 用户模型
│   │   ├── Bookmark.h        # 书签模型
│   │   └── Stats.h           # 统计数据模型
│   ├── repository/            # 数据访问层
│   │   ├── UserRepository.h  # 用户仓库接口
│   │   ├── BookmarkRepository.h # 书签仓库接口
│   │   └── DatabasePool.h    # 数据库连接池
│   ├── service/               # 业务逻辑层
│   │   ├── UserService.h     # 用户服务
│   │   └── BookmarkService.h # 书签服务
│   └── utils/                 # 工具函数
│       └── Utils.h           # 通用工具函数
├── src/                       # 源文件目录
│   ├── auth/
│   │   └── JWT.cpp
│   ├── cache/
│   │   └── LRUCache.cpp
│   ├── config/
│   │   └── Config.cpp
│   ├── http/
│   │   ├── Server.h
│   │   └── Server.cpp
│   ├── models/
│   │   ├── User.cpp
│   │   ├── Bookmark.cpp
│   │   └── Stats.cpp
│   ├── repository/
│   │   ├── UserRepository.cpp
│   │   ├── BookmarkRepository.cpp
│   │   └── DatabasePool.cpp
│   ├── service/
│   │   ├── UserService.cpp
│   │   └── BookmarkService.cpp
│   ├── utils/
│   │   └── Utils.cpp
│   └── main.cpp
├── tests/                     # 单元测试
│   ├── test_user_service.cpp
│   ├── test_bookmark_service.cpp
│   └── test_jwt.cpp
├── config/                    # 配置文件
│   └── config.json
├── CMakeLists.txt
└── README.md
```

## API 接口

### 用户接口

#### 1. 用户注册
- **POST** `/api/users/register`
- 请求体：
```json
{
  "email": "user@example.com",
  "password": "password123",
  "nickname": "Test User"
}
```
- 响应：用户信息

#### 2. 用户登录
- **POST** `/api/users/login`
- 请求体：
```json
{
  "email": "user@example.com",
  "password": "password123"
}
```
- 响应：用户信息 + JWT 令牌

#### 3. 获取当前用户
- **GET** `/api/users/me`
- 认证：需要 `Authorization: Bearer <token>` 头
- 响应：当前用户信息

### 书签接口

#### 1. 创建书签
- **POST** `/api/bookmarks`
- 认证：需要 JWT
- 请求体：
```json
{
  "url": "https://www.example.com",
  "title": "Example Website",
  "description": "This is an example website",
  "tags": ["example", "test"],
  "folder": "personal",
  "is_favorite": false,
  "read_status": "UNREAD"
}
```
- 响应：创建的书签

#### 2. 获取书签
- **GET** `/api/bookmarks/:id`
- 认证：需要 JWT
- 响应：书签信息

#### 3. 更新书签
- **PUT** `/api/bookmarks/:id`
- 认证：需要 JWT
- 请求体：（可选字段）
```json
{
  "title": "Updated Title",
  "description": "Updated description",
  "is_favorite": true
}
```
- 响应：成功状态

#### 4. 删除书签
- **DELETE** `/api/bookmarks/:id`
- 认证：需要 JWT
- 响应：成功状态

#### 5. 查询书签列表
- **GET** `/api/bookmarks`
- 认证：需要 JWT
- 查询参数：
  - `page`: 页码（默认：1）
  - `page_size`: 每页数量（默认：20）
  - `search`: 搜索关键词
  - `tag`: 标签过滤（可多次）
  - `folder`: 文件夹过滤
  - `read_status`: 阅读状态（UNREAD/READ/ARCHIVED）
  - `is_favorite`: 是否收藏（true/false）
  - `sort_by`: 排序字段（created_at/last_accessed）
  - `sort_desc`: 是否降序（true/false）
- 响应：书签列表 + 分页信息

#### 6. 批量操作
- **POST** `/api/bookmarks/batch/read` - 批量标记为已读
- **POST** `/api/bookmarks/batch/unread` - 批量标记为未读
- **POST** `/api/bookmarks/batch/move` - 批量移动到文件夹
- **DELETE** `/api/bookmarks/batch` - 批量删除
- 请求体示例：
```json
{
  "ids": [1, 2, 3],
  "folder": "new-folder" // 仅用于批量移动
}
```

### 统计接口

#### 1. 用户统计
- **GET** `/api/stats`
- 认证：需要 JWT
- 响应：用户统计数据

#### 2. 每日统计
- **GET** `/api/stats/daily?days=14`
- 认证：需要 JWT
- 响应：近 N 天的每日统计

#### 3. 热门域名
- **GET** `/api/stats/domains?limit=10`
- 认证：需要 JWT
- 响应：访问最多的域名列表

### 标签和文件夹接口

#### 1. 获取所有标签
- **GET** `/api/tags`
- 认证：需要 JWT
- 响应：标签列表

#### 2. 重命名标签
- **PUT** `/api/tags`
- 认证：需要 JWT
- 请求体：
```json
{
  "old_tag": "old-name",
  "new_tag": "new-name"
}
```

#### 3. 删除标签
- **DELETE** `/api/tags`
- 认证：需要 JWT
- 请求体：
```json
{
  "tag": "tag-to-delete",
  "remove_from_bookmarks": false
}
```

#### 4. 获取所有文件夹
- **GET** `/api/folders`
- 认证：需要 JWT
- 响应：文件夹列表

### 健康检查

- **GET** `/health`
- 响应：服务健康状态

## 安装和运行

### 1. 依赖安装

```bash
# macOS (Homebrew)
brew install cmake sqlite3 openssl spdlog

# Ubuntu/Debian
sudo apt-get install cmake libsqlite3-dev libssl-dev libspdlog-dev
```

### 2. 构建项目

```bash
mkdir build
cd build
cmake ..
make -j4
```

### 3. 运行测试

```bash
make test
```

### 4. 配置服务

编辑 `config/config.json` 文件：

```json
{
  "database": {
    "path": "./bookmarks.db",
    "max_connections": 10
  },
  "http": {
    "port": 8080,
    "max_threads": 4
  },
  "jwt": {
    "secret_key": "your_secure_secret_key_here",
    "expires_in": 3600
  },
  "cache": {
    "capacity": 1000,
    "ttl": 300
  },
  "debug": false
}
```

### 5. 启动服务

```bash
./build/bookmark-service
```

或者使用自定义配置文件：

```bash
./build/bookmark-service /path/to/config.json
```

服务将在 `http://localhost:8080` 启动。

## 开发指南

### 添加新功能

1. **数据模型**：在 `include/models/` 中添加新模型头文件
2. **数据访问层**：在 `include/repository/` 中添加仓库接口，在 `src/repository/` 中实现
3. **业务逻辑层**：在 `include/service/` 中添加服务接口，在 `src/service/` 中实现
4. **API 接口**：在 `src/http/Server.cpp` 中添加新的路由处理
5. **测试**：在 `tests/` 中添加单元测试

### 代码规范

- 使用 C++17 特性
- 使用智能指针管理内存
- 遵循 RAII 原则
- 使用命名空间组织代码
- 添加适当的错误处理
- 编写单元测试

### 调试

设置 `debug: true` 在配置文件中，将启用更详细的日志输出。

## 性能优化

1. **数据库连接池**：减少频繁的数据库连接开销
2. **索引优化**：在经常查询的字段上创建索引
3. **缓存机制**：LRU 缓存存储频繁访问的数据
4. **批量操作**：支持批量处理减少数据库查询次数
5. **分页查询**：避免一次性加载大量数据

## 安全考虑

1. **密码哈希**：使用 PBKDF2 + SHA256 存储密码
2. **JWT 令牌**：使用安全的密钥和合理的过期时间
3. **输入验证**：所有用户输入都经过验证和 sanitization
4. **SQL 注入防护**：使用预处理语句和参数化查询
5. **XSS 防护**：API 响应进行适当的转义

## 部署

### 生产环境建议

1. **修改 JWT 密钥**：使用强随机密钥
2. **数据库优化**：考虑使用 PostgreSQL 等生产级数据库
3. **HTTPS**：使用 Nginx 等反向代理提供 HTTPS 支持
4. **进程管理**：使用 systemd 或其他进程管理器
5. **监控**：添加日志收集和监控
6. **备份**：定期备份数据库

### Docker 部署

```dockerfile
FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libsqlite3-dev \
    libssl-dev \
    libspdlog-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build && cmake .. && make -j4

EXPOSE 8080

CMD ["./build/bookmark-service"]
```

## 许可证

MIT License

## 贡献

欢迎提交 Issue 和 Pull Request！

## 联系方式

如有问题或建议，请通过以下方式联系：

- 提交 Issue
- 发送邮件

---

*注意：这是一个演示项目，生产环境使用前请进行充分的测试和安全审计。*
