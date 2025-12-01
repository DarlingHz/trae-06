# 个人睡眠与作息追踪后端服务

## 项目简介
这是一个基于C++17的HTTP JSON API后端服务，用于个人睡眠与作息追踪。

## 项目依赖
- C++17及以上编译器
- CMake 3.10及以上
- SQLite3开发库
- nlohmann_json库

## 编译步骤
```bash
mkdir build && cd build
cmake ..
make
```

## 服务启动方式
```bash
./sleep_tracker
```
默认端口为8080。

## 接口调用示例

### 1. 用户注册
```bash
curl -X POST -H "Content-Type: application/json" -d '{"email":"user@example.com","password":"password123","nickname":"Alice","timezone":"Asia/Shanghai"}' http://localhost:8080/api/users/register
```

### 2. 用户登录
```bash
curl -X POST -H "Content-Type: application/json" -d '{"email":"user@example.com","password":"password123"}' http://localhost:8080/api/users/login
```

### 3. 新增睡眠记录
```bash
curl -X POST -H "Content-Type: application/json" -H "Authorization: Bearer <token>" -d '{"start_time":"2025-01-01T23:30:00+08:00","end_time":"2025-01-02T07:30:00+08:00","quality":8,"tags":["工作日","熬夜"],"note":"加班到很晚，入睡较晚"}' http://localhost:8080/api/sleep_sessions
```

### 4. 查询睡眠记录
```bash
curl -H "Authorization: Bearer <token>" http://localhost:8080/api/sleep_sessions?start=2025-01-01&end=2025-01-31&page=1&page_size=20
```

### 5. 获取睡眠统计
```bash
curl -H "Authorization: Bearer <token>" http://localhost:8080/api/stats/summary?start=2025-01-01&end=2025-01-31
```

### 6. 设置睡眠目标
```bash
curl -X POST -H "Content-Type: application/json" -H "Authorization: Bearer <token>" -d '{"goal_hours_per_day":8.0}' http://localhost:8080/api/settings/goal
```

