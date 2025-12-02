# 宠物医院预约与病例管理后端服务

## 项目介绍

这是一个基于C++17开发的宠物医院预约与病例管理后端服务，提供了完整的宠物医院业务功能，包括用户管理、宠物信息管理、医生与科室管理、预约管理以及病例记录管理等。

### 整体架构

本服务采用了分层架构设计，主要分为以下几个层次：

1. **表现层**：负责处理HTTP请求和响应，包括路由注册、请求解析、响应生成等。
2. **业务逻辑层**：负责处理核心业务逻辑，包括用户注册、登录、宠物信息管理、预约管理、病例记录管理等。
3. **数据访问层**：负责与数据库进行交互，包括数据的增删改查等操作。
4. **实体层**：负责定义业务实体，包括用户、宠物、医生、科室、预约、病例记录等。
5. **工具层**：负责提供通用的工具函数和类，包括日志模块、配置模块、JSON解析模块等。

### 技术栈

- **编程语言**：C++17
- **HTTP服务器**：自定义实现的多线程HTTP服务器
- **数据库**：SQLite
- **JSON解析**：nlohmann/json
- **日志**：自定义实现的日志模块
- **配置**：自定义实现的配置模块

## API接口说明

### 1. 用户与认证

#### 1.1 用户注册

- **路径**：`/api/users/register`
- **方法**：`POST`
- **请求参数**：
  ```json
  {
    "email": "string", // 邮箱（必填）
    "password": "string", // 密码（必填）
    "name": "string" // 姓名（必填）
  }
  ```
- **返回参数**：
  ```json
  {
    "id": "integer", // 用户ID
    "email": "string", // 邮箱
    "name": "string", // 姓名
    "created_at": "string" // 创建时间
  }
  ```

#### 1.2 用户登录

- **路径**：`/api/users/login`
- **方法**：`POST`
- **请求参数**：
  ```json
  {
    "email": "string", // 邮箱（必填）
    "password": "string" // 密码（必填）
  }
  ```
- **返回参数**：
  ```json
  {
    "token": "string", // 会话token
    "expires_at": "string", // token过期时间
    "user": {
      "id": "integer", // 用户ID
      "email": "string", // 邮箱
      "name": "string" // 姓名
    }
  }
  ```

#### 1.3 获取用户信息

- **路径**：`/api/users/profile`
- **方法**：`GET`
- **请求头**：`Authorization: Bearer <token>`
- **返回参数**：
  ```json
  {
    "id": "integer", // 用户ID
    "email": "string", // 邮箱
    "name": "string", // 姓名
    "created_at": "string" // 创建时间
  }
  ```

#### 1.4 更新用户信息

- **路径**：`/api/users/profile`
- **方法**：`PUT`
- **请求头**：`Authorization: Bearer <token>`
- **请求参数**：
  ```json
  {
    "name": "string" // 姓名（可选）
  }
  ```
- **返回参数**：
  ```json
  {
    "id": "integer", // 用户ID
    "email": "string", // 邮箱
    "name": "string", // 姓名
    "created_at": "string" // 创建时间
  }
  ```

### 2. 宠物信息管理

#### 2.1 添加宠物

- **路径**：`/api/pets`
- **方法**：`POST`
- **请求头**：`Authorization: Bearer <token>`
- **请求参数**：
  ```json
  {
    "name": "string", // 宠物名称（必填）
    "species": "string", // 宠物种类（必填）
    "breed": "string", // 宠物品种（必填）
    "gender": "string", // 宠物性别（必填）
    "birthday": "string", // 宠物生日（可选）
    "notes": "string" // 备注（可选）
  }
  ```
- **返回参数**：
  ```json
  {
    "id": "integer", // 宠物ID
    "name": "string", // 宠物名称
    "species": "string", // 宠物种类
    "breed": "string", // 宠物品种
    "gender": "string", // 宠物性别
    "birthday": "string", // 宠物生日（可选）
    "notes": "string", // 备注（可选）
    "created_at": "string" // 创建时间
  }
  ```

#### 2.2 获取宠物列表

- **路径**：`/api/pets`
- **方法**：`GET`
- **请求头**：`Authorization: Bearer <token>`
- **请求参数**：
  - `page`：页码（可选，默认1）
  - `page_size`：每页数量（可选，默认10）
- **返回参数**：
  ```json
  {
    "pets": [
      {
        "id": "integer", // 宠物ID
        "name": "string", // 宠物名称
        "species": "string", // 宠物种类
        "breed": "string", // 宠物品种
        "gender": "string", // 宠物性别
        "birthday": "string", // 宠物生日（可选）
        "notes": "string", // 备注（可选）
        "created_at": "string" // 创建时间
      }
    ],
    "page": "integer", // 当前页码
    "page_size": "integer", // 每页数量
    "total": "integer" // 宠物总数
  }
  ```

#### 2.3 获取宠物详情

- **路径**：`/api/pets/:id`
- **方法**：`GET`
- **请求头**：`Authorization: Bearer <token>`
- **路径参数**：`id` - 宠物ID
- **返回参数**：
  ```json
  {
    "id": "integer", // 宠物ID
    "name": "string", // 宠物名称
    "species": "string", // 宠物种类
    "breed": "string", // 宠物品种
    "gender": "string", // 宠物性别
    "birthday": "string", // 宠物生日（可选）
    "notes": "string", // 备注（可选）
    "created_at": "string" // 创建时间
  }
  ```

#### 2.4 更新宠物信息

- **路径**：`/api/pets/:id`
- **方法**：`PUT`
- **请求头**：`Authorization: Bearer <token>`
- **路径参数**：`id` - 宠物ID
- **请求参数**：
  ```json
  {
    "name": "string", // 宠物名称（可选）
    "species": "string", // 宠物种类（可选）
    "breed": "string", // 宠物品种（可选）
    "gender": "string", // 宠物性别（可选）
    "birthday": "string", // 宠物生日（可选）
    "notes": "string" // 备注（可选）
  }
  ```
- **返回参数**：
  ```json
  {
    "id": "integer", // 宠物ID
    "name": "string", // 宠物名称
    "species": "string", // 宠物种类
    "breed": "string", // 宠物品种
    "gender": "string", // 宠物性别
    "birthday": "string", // 宠物生日（可选）
    "notes": "string", // 备注（可选）
    "created_at": "string" // 创建时间
  }
  ```

#### 2.5 删除宠物

- **路径**：`/api/pets/:id`
- **方法**：`DELETE`
- **请求头**：`Authorization: Bearer <token>`
- **路径参数**：`id` - 宠物ID
- **返回参数**：
  ```json
  {
    "message": "string" // 删除成功消息
  }
  ```

### 3. 医生与科室管理

#### 3.1 获取医生列表

- **路径**：`/api/doctors`
- **方法**：`GET`
- **返回参数**：
  ```json
  {
    "doctors": [
      {
        "id": "integer", // 医生ID
        "name": "string", // 医生姓名
        "department_id": "integer", // 科室ID
        "title": "string", // 医生职称（可选）
        "specialty": "string", // 医生擅长（可选）
        "available_time": "string", // 医生可预约时间（可选）
        "created_at": "string" // 创建时间
      }
    ]
  }
  ```

#### 3.2 获取医生详情

- **路径**：`/api/doctors/:id`
- **方法**：`GET`
- **路径参数**：`id` - 医生ID
- **返回参数**：
  ```json
  {
    "id": "integer", // 医生ID
    "name": "string", // 医生姓名
    "department_id": "integer", // 科室ID
    "title": "string", // 医生职称（可选）
    "specialty": "string", // 医生擅长（可选）
    "available_time": "string", // 医生可预约时间（可选）
    "created_at": "string" // 创建时间
  }
  ```

#### 3.3 获取科室列表

- **路径**：`/api/departments`
- **方法**：`GET`
- **返回参数**：
  ```json
  {
    "departments": [
      {
        "id": "integer", // 科室ID
        "name": "string", // 科室名称
        "description": "string" // 科室描述
      }
    ]
  }
  ```

### 4. 预约管理

#### 4.1 创建预约

- **路径**：`/api/appointments`
- **方法**：`POST`
- **请求头**：`Authorization: Bearer <token>`
- **请求参数**：
  ```json
  {
    "pet_id": "integer", // 宠物ID（必填）
    "doctor_id": "integer", // 医生ID（必填）
    "start_time": "string", // 预约开始时间（必填）
    "end_time": "string", // 预约结束时间（必填）
    "reason": "string" // 就诊原因（必填）
  }
  ```
- **返回参数**：
  ```json
  {
    "id": "integer", // 预约ID
    "user_id": "integer", // 用户ID
    "pet_id": "integer", // 宠物ID
    "doctor_id": "integer", // 医生ID
    "start_time": "string", // 预约开始时间
    "end_time": "string", // 预约结束时间
    "reason": "string", // 就诊原因
    "status": "string", // 预约状态（pending/confirmed/completed/canceled）
    "created_at": "string" // 创建时间
  }
  ```

#### 4.2 获取我的预约

- **路径**：`/api/appointments/my`
- **方法**：`GET`
- **请求头**：`Authorization: Bearer <token>`
- **请求参数**：
  - `from`：开始时间（可选）
  - `to`：结束时间（可选）
- **返回参数**：
  ```json
  {
    "appointments": [
      {
        "id": "integer", // 预约ID
        "user_id": "integer", // 用户ID
        "pet_id": "integer", // 宠物ID
        "doctor_id": "integer", // 医生ID
        "start_time": "string", // 预约开始时间
        "end_time": "string", // 预约结束时间
        "reason": "string", // 就诊原因
        "status": "string", // 预约状态
        "created_at": "string" // 创建时间
      }
    ]
  }
  ```

#### 4.3 获取医生预约

- **路径**：`/api/appointments/doctor/:id`
- **方法**：`GET`
- **请求头**：`Authorization: Bearer <token>`
- **路径参数**：`id` - 医生ID
- **请求参数**：
  - `date`：日期（可选）
- **返回参数**：
  ```json
  {
    "appointments": [
      {
        "id": "integer", // 预约ID
        "user_id": "integer", // 用户ID
        "pet_id": "integer", // 宠物ID
        "doctor_id": "integer", // 医生ID
        "start_time": "string", // 预约开始时间
        "end_time": "string", // 预约结束时间
        "reason": "string", // 就诊原因
        "status": "string", // 预约状态
        "created_at": "string" // 创建时间
      }
    ]
  }
  ```

#### 4.4 取消预约

- **路径**：`/api/appointments/:id/cancel`
- **方法**：`POST`
- **请求头**：`Authorization: Bearer <token>`
- **路径参数**：`id` - 预约ID
- **返回参数**：
  ```json
  {
    "message": "string" // 取消成功消息
  }
  ```

### 5. 病例记录

#### 5.1 创建病例记录

- **路径**：`/api/records`
- **方法**：`POST`
- **请求头**：`Authorization: Bearer <token>`
- **请求参数**：
  ```json
  {
    "appointment_id": "integer", // 预约ID（必填）
    "chief_complaint": "string", // 主诉（必填）
    "diagnosis": "string", // 诊断结论（必填）
    "treatment": "string", // 用药建议（必填）
    "notes": "string" // 医生备注（可选）
  }
  ```
- **返回参数**：
  ```json
  {
    "id": "integer", // 病例记录ID
    "appointment_id": "integer", // 预约ID
    "chief_complaint": "string", // 主诉
    "diagnosis": "string", // 诊断结论
    "treatment": "string", // 用药建议
    "notes": "string", // 医生备注（可选）
    "created_at": "string", // 创建时间
    "updated_at": "string" // 更新时间
  }
  ```

#### 5.2 获取宠物病例记录

- **路径**：`/api/records/pet/:id`
- **方法**：`GET`
- **请求头**：`Authorization: Bearer <token>`
- **路径参数**：`id` - 宠物ID
- **请求参数**：
  - `page`：页码（可选，默认1）
  - `page_size`：每页数量（可选，默认10）
- **返回参数**：
  ```json
  {
    "records": [
      {
        "id": "integer", // 病例记录ID
        "appointment_id": "integer", // 预约ID
        "chief_complaint": "string", // 主诉
        "diagnosis": "string", // 诊断结论
        "treatment": "string", // 用药建议
        "notes": "string", // 医生备注（可选）
        "created_at": "string", // 创建时间
        "updated_at": "string" // 更新时间
      }
    ],
    "page": "integer", // 当前页码
    "page_size": "integer", // 每页数量
    "total": "integer" // 病例记录总数
  }
  ```

## 编译与运行步骤

### 1. 环境准备

- **操作系统**：Linux/macOS
- **编译器**：GCC 9.0+ 或 Clang 10.0+
- **CMake**：3.15+
- **SQLite**：3.0+

### 2. 编译步骤

1. 克隆或下载项目代码到本地

2. 进入项目根目录

3. 创建build目录并进入
   ```bash
   mkdir build && cd build
   ```

4. 运行CMake生成编译文件
   ```bash
   cmake ..
   ```

5. 运行make编译项目
   ```bash
   make -j$(nproc)
   ```

6. 编译完成后，会在build目录下生成可执行文件`pet_hospital_server`

### 3. 运行步骤

1. 确保当前目录为项目根目录

2. 运行可执行文件
   ```bash
   ./build/pet_hospital_server
   ```

3. 服务器启动后，会监听配置文件中指定的端口（默认8080）

4. 可以使用curl或其他HTTP客户端工具来测试API接口

### 4. 数据库初始化

服务器启动时会自动检查数据库表是否存在，如果不存在则会自动创建所有需要的表。因此，不需要手动执行数据库初始化命令。

## 使用示例

以下是使用curl命令测试完整业务流程的示例：

### 1. 用户注册

```bash
curl -X POST -H "Content-Type: application/json" -d '{"email":"test@example.com","password":"123456","name":"Test User"}' http://localhost:8080/api/users/register
```

### 2. 用户登录

```bash
curl -X POST -H "Content-Type: application/json" -d '{"email":"test@example.com","password":"123456"}' http://localhost:8080/api/users/login
```

登录成功后，会返回一个会话token，例如：

```json
{
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "expires_at": "2023-12-03T10:30:00Z",
  "user": {
    "id": 1,
    "email": "test@example.com",
    "name": "Test User"
  }
}
```

在后续的请求中，需要将token放在Authorization请求头中，例如：

```bash
curl -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..." http://localhost:8080/api/users/profile
```

为了方便后续操作，我们可以将token保存到一个环境变量中：

```bash
export TOKEN="eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
```

### 3. 新建宠物

```bash
curl -X POST -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d '{"name":"Tom","species":"Cat","breed":"Persian","gender":"Male","birthday":"2020-01-01","notes":"A cute Persian cat"}' http://localhost:8080/api/pets
```

### 4. 新建预约

首先，我们需要获取医生列表，选择一个医生：

```bash
curl http://localhost:8080/api/doctors
```

假设我们选择医生ID为1的医生，然后创建预约：

```bash
curl -X POST -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d '{"pet_id":1,"doctor_id":1,"start_time":"2023-12-04T10:00:00Z","end_time":"2023-12-04T11:00:00Z","reason":"Routine checkup"}' http://localhost:8080/api/appointments
```

### 5. 完成预约

目前，系统中还没有提供直接完成预约的API接口。在实际应用中，完成预约通常是由医生或管理员来操作的。因此，我们可以假设预约已经完成，然后创建病例记录。

### 6. 新建病例记录

首先，我们需要获取预约ID：

```bash
curl -H "Authorization: Bearer $TOKEN" http://localhost:8080/api/appointments/my
```

假设预约ID为1，然后创建病例记录：

```bash
curl -X POST -H "Authorization: Bearer $TOKEN" -H "Content-Type: application/json" -d '{"appointment_id":1,"chief_complaint":"Loss of appetite","diagnosis":"Gastrointestinal infection","treatment":"Antibiotics for 7 days","notes":"Patient should rest and eat light food"}' http://localhost:8080/api/records
```

### 7. 查询病例

```bash
curl -H "Authorization: Bearer $TOKEN" http://localhost:8080/api/records/pet/1
```

## 待改进

1. **性能优化**：
   - 实现数据库连接池，提高数据库访问性能
   - 优化HTTP服务器的线程池实现，提高并发处理能力
   - 实现更高效的缓存机制，减少数据库访问次数

2. **安全增强**：
   - 实现更安全的密码哈希算法（如bcrypt、Argon2等）
   - 实现token的刷新机制，提高系统安全性
   - 实现API接口的限流机制，防止恶意攻击

3. **功能完善**：
   - 实现医生排班管理功能
   - 实现预约提醒功能
   - 实现病例记录的导出功能
   - 实现系统管理功能（如用户权限管理、日志管理等）

4. **代码质量**：
   - 实现更完善的单元测试
   - 实现代码覆盖率统计
   - 实现更规范的代码风格

## 许可证

MIT License
