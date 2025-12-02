# 家庭餐食计划与菜谱管理系统

一个基于C++17的后端服务，用于管理家庭餐食计划和菜谱。

## 功能特性

### 用户管理
- 用户注册
- 用户登录
- 基于token的身份认证

### 菜谱管理
- 创建菜谱
- 查看菜谱列表
- 查看单个菜谱详情
- 更新菜谱
- 标记菜谱为收藏
- 删除菜谱（逻辑删除）

### 餐食计划管理
- 创建或更新餐食计划
- 查看餐食计划
- 删除餐食计划

### 购物清单生成
- 根据指定日期区间内的餐食计划生成购物清单
- 自动聚合相同食材的数量

## 技术栈

- **C++17**：编程语言
- **Crow**：C++ Web框架
- **SQLite3**：数据库
- **nlohmann_json**：JSON序列化/反序列化库
- **Catch2**：单元测试框架
- **CMake**：构建系统

## 目录结构

```
meal_plan_manager/
├── include/              # 头文件
│   ├── database.hpp     # 数据库连接管理
│   ├── user_service.hpp # 用户服务
│   ├── recipe_service.hpp # 菜谱服务
│   ├── meal_plan_service.hpp # 餐食计划服务
│   └── shopping_list_service.hpp # 购物清单服务
├── src/                  # 源文件
│   ├── main.cpp         # 主程序
│   ├── database.cpp     # 数据库连接管理实现
│   ├── user_service.cpp # 用户服务实现
│   ├── recipe_service.cpp # 菜谱服务实现
│   ├── meal_plan_service.cpp # 餐食计划服务实现
│   └── shopping_list_service.cpp # 购物清单服务实现
├── tests/                # 单元测试
│   └── shopping_list_test.cpp # 购物清单生成测试
├── config/               # 配置文件
│   └── config.json      # 服务配置
├── third_party/          # 第三方库
│   ├── crow/             # Crow Web框架
│   └── nlohmann_json/   # JSON库
├── build/                # 构建目录
├── data/                 # 数据库文件目录
├── CMakeLists.txt        # CMake配置
└── README.md             # 项目说明
```

## 构建与运行

### 环境要求
- C++17编译器（如GCC 8+、Clang 7+）
- CMake 3.15+
- SQLite3

### 安装依赖

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y cmake g++ sqlite3 libsqlite3-dev
```

#### macOS
```bash
brew install cmake g++ sqlite3
```

### 构建项目

```bash
# 创建构建目录
mkdir -p build
cd build

# 配置CMake
cmake ..

# 构建项目
make -j$(nproc)
```

### 运行服务

```bash
# 确保数据目录存在
mkdir -p data

# 运行服务
./meal_plan_manager
```

服务将在端口8080上启动。

## API接口文档

### 认证方式

所有需要用户身份的接口，都需要在请求头中携带`X-Auth-Token`字段，该字段的值为用户登录时返回的token。

### 错误响应格式

当接口出现错误时，将返回以下格式的JSON响应：

```json
{
  "errorCode": "错误代码",
  "message": "错误描述"
}
```

### 用户接口

#### 注册用户

```
POST /api/users/register
Content-Type: application/json

{
  "name": "用户名",
  "email": "邮箱",
  "password": "密码"
}
```

响应：

```json
{
  "userId": 1
}
```

#### 用户登录

```
POST /api/users/login
Content-Type: application/json

{
  "email": "邮箱",
  "password": "密码"
}
```

响应：

```json
{
  "token": "登录token"
}
```

### 菜谱接口

#### 创建菜谱

```
POST /api/recipes
Content-Type: application/json
X-Auth-Token: 登录token

{
  "title": "菜谱标题",
  "description": "菜谱描述",
  "servings": 4,
  "tags": ["家常菜", "低脂"],
  "ingredients": [
    {
      "name": "鸡蛋",
      "quantity": 2,
      "unit": "个"
    },
    {
      "name": "牛奶",
      "quantity": 250,
      "unit": "ml"
    }
  ],
  "steps": [
    "1. 热锅",
    "2. 打入鸡蛋",
    "3. 煎至两面金黄"
  ]
}
```

响应：

```json
{
  "recipeId": 1
}
```

#### 查看菜谱列表

```
GET /api/recipes?keyword=鸡蛋&tag=早餐&ingredient=牛奶
X-Auth-Token: 登录token
```

参数：
- `keyword`：在标题/描述中模糊匹配
- `tag`：根据标签过滤
- `ingredient`：根据包含某个食材过滤

响应：

```json
{
  "recipes": [
    {
      "id": 1,
      "title": "煎蛋",
      "description": "简单的煎蛋",
      "servings": 1,
      "tags": ["早餐", "快手菜"],
      "is_favorite": false,
      "created_at": "2025-01-06T10:00:00Z",
      "updated_at": "2025-01-06T10:00:00Z"
    }
  ]
}
```

#### 查看单个菜谱详情

```
GET /api/recipes/1
X-Auth-Token: 登录token
```

响应：

```json
{
  "id": 1,
  "title": "煎蛋",
  "description": "简单的煎蛋",
  "servings": 1,
  "tags": ["早餐", "快手菜"],
  "ingredients": [
    {
      "name": "鸡蛋",
      "quantity": 2,
      "unit": "个"
    },
    {
      "name": "牛奶",
      "quantity": 250,
      "unit": "ml"
    }
  ],
  "steps": [
    "1. 热锅",
    "2. 打入鸡蛋",
    "3. 煎至两面金黄"
  ],
  "is_favorite": false,
  "created_at": "2025-01-06T10:00:00Z",
  "updated_at": "2025-01-06T10:00:00Z"
}
```

#### 更新菜谱

```
PUT /api/recipes/1
Content-Type: application/json
X-Auth-Token: 登录token

{
  "title": "煎蛋（更新）",
  "description": "简单的煎蛋（更新）",
  "servings": 2,
  "tags": ["早餐", "快手菜", "低脂"],
  "ingredients": [
    {
      "name": "鸡蛋",
      "quantity": 4,
      "unit": "个"
    },
    {
      "name": "牛奶",
      "quantity": 500,
      "unit": "ml"
    }
  ],
  "steps": [
    "1. 热锅",
    "2. 打入鸡蛋",
    "3. 煎至两面金黄",
    "4. 加入牛奶"
  ]
}
```

响应：

```json
{
  "success": true
}
```

#### 标记菜谱为收藏

```
POST /api/recipes/1/favorite
Content-Type: application/json
X-Auth-Token: 登录token

{
  "favorite": true
}
```

响应：

```json
{
  "success": true
}
```

#### 删除菜谱

```
DELETE /api/recipes/1
X-Auth-Token: 登录token
```

响应：

```json
{
  "success": true
}
```

### 餐食计划接口

#### 创建或更新餐食计划

```
POST /api/meal-plans
Content-Type: application/json
X-Auth-Token: 登录token

{
  "weekStartDate": "2025-01-06",
  "entries": [
    {
      "date": "2025-01-06",
      "slot": "breakfast",
      "recipeId": 1
    },
    {
      "date": "2025-01-06",
      "slot": "dinner",
      "recipeId": 2
    }
  ]
}
```

参数：
- `weekStartDate`：一周的开始日期（周一），格式为YYYY-MM-DD
- `entries`：餐食计划条目数组
  - `date`：具体日期，格式为YYYY-MM-DD
  - `slot`：餐食时段，可选值为breakfast、lunch、dinner、snack
  - `recipeId`：菜谱ID

响应：

```json
{
  "success": true
}
```

#### 查看餐食计划

```
GET /api/meal-plans?weekStartDate=2025-01-06
X-Auth-Token: 登录token
```

参数：
- `weekStartDate`：一周的开始日期（周一），格式为YYYY-MM-DD

响应：

```json
{
  "meal_plan": {
    "id": 1,
    "week_start_date": "2025-01-06",
    "entries": [
      {
        "date": "2025-01-06",
        "slot": "breakfast",
        "recipeId": 1,
        "recipe": {
          "id": 1,
          "title": "煎蛋",
          "tags": ["早餐", "快手菜"]
        }
      },
      {
        "date": "2025-01-06",
        "slot": "dinner",
        "recipeId": 2,
        "recipe": {
          "id": 2,
          "title": "红烧肉",
          "tags": ["晚餐", "家常菜"]
        }
      }
    ],
    "created_at": "2025-01-06T10:00:00Z",
    "updated_at": "2025-01-06T10:00:00Z"
  }
}
```

#### 删除餐食计划

```
DELETE /api/meal-plans?weekStartDate=2025-01-06
X-Auth-Token: 登录token
```

参数：
- `weekStartDate`：一周的开始日期（周一），格式为YYYY-MM-DD

响应：

```json
{
  "success": true
}
```

### 购物清单接口

#### 生成购物清单

```
GET /api/meal-plans/shopping-list?from=2025-01-06&to=2025-01-12
X-Auth-Token: 登录token
```

参数：
- `from`：开始日期，格式为YYYY-MM-DD
- `to`：结束日期，格式为YYYY-MM-DD

响应：

```json
{
  "shopping_list": [
    {
      "name": "鸡蛋",
      "quantity": 6,
      "unit": "个"
    },
    {
      "name": "牛奶",
      "quantity": 750,
      "unit": "ml"
    },
    {
      "name": "猪肉",
      "quantity": 500,
      "unit": "克"
    }
  ]
}
```

## 配置文件

配置文件位于`config/config.json`，内容如下：

```json
{
  "port": 8080,
  "database": {
    "path": "data/meal_plan_manager.db"
  },
  "log": {
    "level": "info"
  }
}
```

配置项说明：
- `port`：服务监听的端口
- `database.path`：数据库文件的路径
- `log.level`：日志级别，可选值为debug、info、warn、error

## 数据库结构

### 用户表（users）

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | INTEGER | 主键，自增 |
| name | TEXT | 用户名 |
| email | TEXT | 邮箱，唯一 |
| password | TEXT | 密码（明文存储） |
| created_at | INTEGER | 创建时间戳 |
| updated_at | INTEGER | 更新时间戳 |

### 菜谱表（recipes）

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | INTEGER | 主键，自增 |
| owner_user_id | INTEGER | 所属用户ID，外键 |
| title | TEXT | 菜谱标题 |
| description | TEXT | 菜谱描述 |
| servings | INTEGER | 推荐食用人数 |
| tags | TEXT | 标签，JSON数组 |
| ingredients | TEXT | 食材，JSON数组 |
| steps | TEXT | 步骤，JSON数组 |
| is_favorite | INTEGER | 是否收藏（0: 否，1: 是） |
| is_archived | INTEGER | 是否归档（0: 否，1: 是） |
| created_at | INTEGER | 创建时间戳 |
| updated_at | INTEGER | 更新时间戳 |

### 餐食计划表（meal_plans）

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | INTEGER | 主键，自增 |
| user_id | INTEGER | 用户ID，外键 |
| week_start_date | TEXT | 一周的开始日期（周一） |
| entries | TEXT | 餐食计划条目，JSON数组 |
| created_at | INTEGER | 创建时间戳 |
| updated_at | INTEGER | 更新时间戳 |

### 用户token表（user_tokens）

| 字段名 | 类型 | 说明 |
|--------|------|------|
| id | INTEGER | 主键，自增 |
| user_id | INTEGER | 用户ID，外键 |
| token | TEXT | 登录token |
| expires_at | INTEGER | 过期时间戳 |
| created_at | INTEGER | 创建时间戳 |

## 安全注意事项

1. **密码存储**：当前版本中，用户密码以明文形式存储在数据库中，这在生产环境中是不安全的。建议在生产环境中使用密码哈希算法（如bcrypt）来存储密码。

2. **Token安全**：登录token以明文形式存储在数据库中，建议在生产环境中使用加密算法来存储token。

3. **SQL注入**：当前版本中，已经使用了参数化查询来防止SQL注入攻击，但仍建议在生产环境中进一步加强安全措施。

4. **HTTPS**：当前版本中，服务使用HTTP协议进行通信，建议在生产环境中使用HTTPS协议来加密通信内容。

## 开发说明

### 添加新功能

1. 在`include/`目录下创建新的服务头文件
2. 在`src/`目录下创建新的服务实现文件
3. 在`src/main.cpp`中注册新的API路由
4. 更新`CMakeLists.txt`文件，添加新的源文件

### 运行单元测试

```bash
cd build
make shopping_list_test
./shopping_list_test
```

### 生成文档

可以使用Doxygen来生成项目文档：

```bash
doxygen -g Doxyfile
# 编辑Doxyfile文件，配置文档生成选项
doxygen Doxyfile
```

## 许可证

MIT License
