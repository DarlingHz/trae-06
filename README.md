# 企业合同审批流程管理后端服务

一个基于 C++ 开发的企业合同审批流程管理系统，提供完整的合同生命周期管理功能。

## 技术栈

- **C++17**: 核心开发语言
- **CMake**: 项目构建工具
- **SQLite**: 轻量级数据库持久化
- **cpp-httplib**: 单文件 HTTP 服务器库
- **nlohmann/json**: JSON 处理库
- **fmt**: 现代化格式化库
- **Catch2**: 单元测试框架

## 项目结构

```
project-root/
├── CMakeLists.txt              # CMake 构建配置
├── main.cpp                    # 程序入口
├── domain/                    # 领域模型
│   ├── user.h/cpp            # 用户数据结构
│   ├── contract.h/cpp        # 合同数据结构
│   ├── approval_step.h/cpp   # 审批步骤数据结构
│   └── approval_log.h/cpp    # 审批日志数据结构
├── storage/                   # 存储层
│   ├── storage_interface.h   # 存储接口定义
│   └── sqlite_storage.h/cpp  # SQLite 存储实现
├── service/                   # 服务层
│   ├── contract_service.h/cpp # 合同业务逻辑
├── http/                     # HTTP 接口层
│   ├── handler.h/cpp         # HTTP 请求处理
├── tests/                    # 单元测试
│   └── contract_service_test.cpp
├── third_party/              # 第三方库
│   ├── cpp-httplib/
│   ├── nlohmann_json/
│   ├── fmt/
│   └── catch2/
└── data/                     # 数据库文件目录
    └── contracts.db
```

## 构建项目

### 前提条件

- C++17 编译器 (GCC 8+, Clang 6+, MSVC 2019+)
- CMake 3.15+

### 构建步骤

1. 创建 build 目录
```bash
mkdir build
cd build
```

2. 配置 CMake
```bash
cmake ..
```

3. 编译项目
```bash
make -j4
```

编译完成后，可执行文件将生成在 `build/contract_server`

## 运行服务

### 基本运行

```bash
./build/contract_server
```

### 命令行参数

- `--port <port>`: 指定服务监听端口（默认 8080）
- `--db <path>`: 指定 SQLite 数据库文件路径（默认 ./data/contracts.db）

### 示例

```bash
./build/contract_server --port 8080 --db ./data/contracts.db
```

## HTTP API 接口

所有请求需要在头部携带 `X-User-Id` 标识当前用户。

### 1. 创建合同

```bash
curl -X POST http://localhost:8080/contracts \
  -H "X-User-Id: 1" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "年度采购合同",
    "counterparty": "供应商A",
    "amount": 150000,
    "currency": "CNY",
    "department": "sales"
  }'
```

### 2. 编辑合同

```bash
curl -X PUT http://localhost:8080/contracts/1 \
  -H "X-User-Id: 1" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "年度采购合同 - 修订版",
    "counterparty": "供应商A",
    "amount": 150000,
    "currency": "CNY",
    "department": "sales"
  }'
```

### 3. 提交合同审批

```bash
curl -X POST http://localhost:8080/contracts/1/submit \
  -H "X-User-Id: 1"
```

### 4. 获取合同详情

```bash
curl -H "X-User-Id: 1" http://localhost:8080/contracts/1
```

### 5. 查询合同列表

```bash
# 获取所有合同
curl -H "X-User-Id: 1" http://localhost:8080/contracts

# 分页查询
curl -H "X-User-Id: 1" "http://localhost:8080/contracts?page=1&page_size=10"

# 按状态查询
curl -H "X-User-Id: 1" "http://localhost:8080/contracts?status=approving"

# 按金额范围查询
curl -H "X-User-Id: 1" "http://localhost:8080/contracts?min_amount=10000&max_amount=100000"
```

### 6. 获取待审批合同

```bash
curl -H "X-User-Id: 2" http://localhost:8080/contracts/pending
```

### 7. 审批合同

```bash
# 批准合同
curl -X POST http://localhost:8080/contracts/1/approve \
  -H "X-User-Id: 2" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "approve",
    "comment": "同意审批"
  }'

# 拒绝合同
curl -X POST http://localhost:8080/contracts/1/approve \
  -H "X-User-Id: 2" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "reject",
    "comment": "需要进一步确认"
  }'

# 转办合同
curl -X POST http://localhost:8080/contracts/1/approve \
  -H "X-User-Id: 2" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "transfer",
    "comment": "请经理审批",
    "transfer_to_user_id": 3
  }'
```

### 8. 获取审批历史

```bash
curl -H "X-User-Id: 1" http://localhost:8080/contracts/1/history
```

### 9. 取消合同

```bash
curl -X POST http://localhost:8080/contracts/1/cancel \
  -H "X-User-Id: 1"
```

## 单元测试

### 编译测试

测试已与主项目一起编译，可执行文件为 `build/contract_service_test`

### 运行测试

```bash
./build/contract_service_test
```

### 测试覆盖

- 合同创建与提交流程
- 完整审批通过流程
- 审批拒绝流程

## 数据库设计

### 1. users 表
- id: INTEGER PRIMARY KEY
- name: TEXT
- department: TEXT
- role: TEXT

### 2. contracts 表
- id: INTEGER PRIMARY KEY
- title: TEXT
- counterparty: TEXT
- amount: INTEGER
- currency: TEXT
- creator_id: INTEGER
- department: TEXT
- status: TEXT
- created_at: TEXT
- updated_at: TEXT

### 3. approval_steps 表
- id: INTEGER PRIMARY KEY
- contract_id: INTEGER
- step_order: INTEGER
- role: TEXT
- approver_id: INTEGER
- status: TEXT
- comment: TEXT
- acted_at: TEXT

### 4. approval_logs 表
- id: INTEGER PRIMARY KEY
- contract_id: INTEGER
- step_id: INTEGER
- operator_id: INTEGER
- action: TEXT
- comment: TEXT
- created_at: TEXT

## 业务规则

### 合同状态流转
- **draft**（草稿）：新建合同时的初始状态
- **submitted**（已提交）：提交后进入审批流程
- **approving**（审批中）：审批步骤进行中
- **approved**（已批准）：所有步骤审批通过
- **rejected**（已拒绝）：任意步骤被拒绝
- **cancelled**（已取消）：创建人可取消草稿或已拒绝的合同

### 审批流程规则
- 金额 < 50,000：需要部门 manager 审批 1 步
- 50,000 ≤ 金额 < 200,000：依次为 manager → finance_approver
- 金额 ≥ 200,000：依次为 manager → finance_approver → legal_approver

## 许可证

MIT License
