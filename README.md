# 协作文档版本与评论管理后端服务

这是一个使用C++17实现的协作文档版本与评论管理后端服务。该服务提供HTTP/JSON接口，用于管理用户、文档、文档版本以及评论。

## 项目特性

- **用户管理**：支持创建用户和查询用户信息
- **文档管理**：支持创建文档、查询文档列表、获取文档详情
- **文档版本管理**：支持创建文档版本、查询文档版本列表、获取指定版本内容
- **评论管理**：支持为文档或版本添加评论、查询评论列表
- **统计功能**：支持获取服务的统计信息
- **健康检查**：支持服务健康状态检查
- **性能优化**：实现了LRU缓存，用于缓存最近访问的文档版本内容
- **并发处理**：支持多线程处理HTTP请求
- **配置文件**：支持通过配置文件设置服务参数

## 技术栈

- **C++17**：编程语言
- **CMake**：构建系统
- **SQLite3**：数据库
- **cpp-httplib**：HTTP服务器库
- **nlohmann/json**：JSON序列化/反序列化库

## 构建依赖

- C++17或以上标准
- CMake 3.10或以上版本
- SQLite3开发库
- cpp-httplib库（单头文件，已包含在项目中）
- nlohmann/json库（单头文件，已包含在项目中）

## 编译步骤

1. 创建构建目录：
```bash
mkdir build
cd build
```

2. 生成Makefile：
```bash
cmake ..
```

3. 编译项目：
```bash
make
```

编译完成后，将生成可执行文件`collab_doc_service`。

## 启动服务

在构建目录中执行以下命令启动服务：

```bash
./collab_doc_service
```

服务将根据`config/config.json`文件中的配置启动。默认配置为：
- 端口：8080
- 最大线程数：4
- 缓存容量：100
- 数据库路径：./data/collab_doc.db

## 示例请求

以下是一些示例curl请求，用于测试服务的功能。

### 1. 创建用户并创建文档

```bash
# 创建用户
curl -X POST http://localhost:8080/users \
  -H "Content-Type: application/json" \
  -d '{"name": "John Doe", "email": "john@example.com"}'

# 创建文档（假设用户ID为1）
curl -X POST http://localhost:8080/documents \
  -H "Content-Type: application/json" \
  -d '{"owner_id": 1, "title": "My First Document", "tags": ["test", "example"]}'
```

### 2. 创建新版本并获取指定版本内容

```bash
# 为文档创建新版本（假设文档ID为1）
curl -X POST http://localhost:8080/documents/1/versions \
  -H "Content-Type: application/json" \
  -d '{"content": "This is the first version of the document."}'

# 获取文档的指定版本内容（假设文档ID为1，版本号为1）
curl -X GET http://localhost:8080/documents/1/versions/1
```

### 3. 添加评论并查询评论列表

```bash
# 为文档添加评论（假设文档ID为1，用户ID为1）
curl -X POST http://localhost:8080/documents/1/comments \
  -H "Content-Type: application/json" \
  -d '{"author_id": 1, "content": "This is a great document!", "version_number": 1}'

# 查询文档的评论列表（假设文档ID为1）
curl -X GET http://localhost:8080/documents/1/comments
```

### 4. 获取统计信息

```bash
# 获取服务的统计信息
curl -X GET http://localhost:8080/metrics
```

### 5. 健康检查

```bash
# 检查服务健康状态
curl -X GET http://localhost:8080/healthz
```

## 项目结构

```
.
├── CMakeLists.txt          # CMake构建配置文件
├── README.md                # 项目说明文档
├── config/                  # 配置文件目录
│   └── config.json          # 服务配置文件
├── include/                 # 头文件目录
│   ├── config.h             # 配置类头文件
│   ├── data_models.h        # 数据模型类头文件
│   ├── database.h           # 数据库类头文件
│   ├── lru_cache.h          # LRU缓存类头文件
│   ├── lru_cache.tpp        # LRU缓存类模板实现
│   ├── router.h              # 路由类头文件
│   └── service.h             # 业务逻辑类头文件
├── src/                     # 源文件目录
│   ├── config.cpp            # 配置类实现
│   ├── database.cpp          # 数据库类实现
│   ├── main.cpp              # 程序入口文件
│   ├── router.cpp            # 路由类实现
│   └── service.cpp           # 业务逻辑类实现
├── scripts/                  # 脚本文件目录
│   └── init_db.sql           # 数据库初始化脚本
└── tests/                    # 测试文件目录（预留）
```

## 配置文件说明

配置文件`config/config.json`包含以下参数：

```json
{
  "port": 8080,
  "max_threads": 4,
  "cache_capacity": 100,
  "database_path": "./data/collab_doc.db"
}
```

- `port`：服务监听的端口
- `max_threads`：服务使用的最大线程数
- `cache_capacity`：LRU缓存的容量
- `database_path`：SQLite数据库文件的路径

## 数据库设计

数据库包含以下表：

- `users`：用户表
- `documents`：文档表
- `document_versions`：文档版本表
- `comments`：评论表

数据库初始化脚本位于`scripts/init_db.sql`。

## 缓存设计

实现了一个线程安全的LRU缓存，用于缓存最近访问的文档版本内容。缓存的key是`(document_id, version_number)`的组合，value是文档版本的内容。

当访问文档的指定版本时，服务会先从缓存中读取内容，如果缓存未命中，则从数据库中读取内容并写入缓存。

## 并发处理

HTTP服务使用cpp-httplib库实现，支持多线程处理请求。服务会根据配置文件中的`max_threads`参数设置最大线程数。

对共享资源（如缓存、数据库连接）的访问进行了适当的同步，避免数据竞争。

## 错误处理

所有接口都返回JSON格式的响应，包含清晰的错误信息。错误响应包含以下字段：

- `error_code`：错误代码
- `message`：错误信息

常见的错误代码包括：
- 1000：无效的JSON格式
- 1001：缺少必需的参数
- 1002：创建用户失败
- 2001：缺少必需的参数
- 2002：创建文档失败
- 2003：文档不存在
- 2004：无效的文档ID
- 3001：缺少必需的参数
- 3002：创建文档版本失败
- 3003：文档版本不存在
- 3004：无效的文档ID或版本号
- 4001：缺少必需的参数
- 4002：创建评论失败
- 500：内部服务器错误

## 扩展功能

可以根据需要扩展以下功能：

- 用户认证和授权
- 文档权限管理
- 文档内容搜索
- 文档版本对比
- 评论回复功能
- 服务监控和日志
- 数据备份和恢复

## 许可证

MIT License
