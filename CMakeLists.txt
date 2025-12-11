cmake_minimum_required(VERSION 3.14)
project(trae-06 VERSION 1.0.0 LANGUAGES CXX)

# C++ 标准要求
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# 查找依赖
find_package(Threads REQUIRED)

# 第三方库（使用 FetchContent 自动下载）
include(FetchContent)

# 1. HTTP服务器：cpp-httplib
add_subdirectory(third_party/cpp-httplib-0.14.3)
include_directories(third_party/cpp-httplib-0.14.3)

# 2. JSON 处理：nlohmann/json
add_subdirectory(third_party/json-3.11.3)
include_directories(third_party/json-3.11.3/include)

# 3. 数据库：SQLite3
find_package(SQLite3 REQUIRED)
include_directories(${SQLite3_INCLUDE_DIRS})

# 4. 日志：spdlog
find_package(spdlog REQUIRED)
include_directories(${spdlog_INCLUDE_DIRS})

# 5. 测试框架：Catch2
find_package(Catch2 REQUIRED)
include_directories(${Catch2_INCLUDE_DIRS})

# 项目模块
add_subdirectory(src)
add_subdirectory(tests)