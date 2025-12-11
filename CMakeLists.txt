cmake_minimum_required(VERSION 3.10)
project(CollabDocService VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找依赖库
find_package(SQLite3 REQUIRED)

# 添加头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)

# 收集源文件
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# 创建可执行文件
add_executable(collab_doc_service ${SOURCES})

# 链接依赖库
target_link_libraries(
    collab_doc_service
    ${SQLite3_LIBRARIES}
    pthread
)

# 复制配置文件到构建目录
configure_file(
    ${PROJECT_SOURCE_DIR}/config/config.json
    ${PROJECT_BINARY_DIR}/config.json
    COPYONLY
)

# 复制数据库初始化脚本到构建目录
configure_file(
    ${PROJECT_SOURCE_DIR}/scripts/init_db.sql
    ${PROJECT_BINARY_DIR}/init_db.sql
    COPYONLY
)

# 创建数据目录
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data)