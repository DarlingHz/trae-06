cmake_minimum_required(VERSION 3.14)
project(event-signup-service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 查找依赖包
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(cpp-httplib REQUIRED)

# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${cpp-httplib_INCLUDE_DIRS}
)

# 收集源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接依赖库
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Threads::Threads
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
    cpp-httplib::cpp-httplib
)

# 安装目标
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# 复制配置文件到构建目录
configure_file(
    ${CMAKE_SOURCE_DIR}/config/config.json
    ${CMAKE_BINARY_DIR}/config/config.json
    COPYONLY
)

# 复制数据库脚本到构建目录
configure_file(
    ${CMAKE_SOURCE_DIR}/db/schema.sql
    ${CMAKE_BINARY_DIR}/db/schema.sql
    COPYONLY
)
