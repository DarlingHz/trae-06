cmake_minimum_required(VERSION 3.16)
project(announcement_server VERSION 1.0.0 LANGUAGES CXX)

# C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 查找依赖
find_package(SQLite3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${SQLite3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# 源文件
set(SOURCES
    src/main.cpp
    src/config/config.cpp
    src/controllers/auth_controller.cpp
    src/controllers/announcement_controller.cpp
    src/controllers/stats_controller.cpp
    src/controllers/health_controller.cpp
    src/services/auth_service.cpp
    src/services/announcement_service.cpp
    src/services/stats_service.cpp
    src/repositories/user_repository.cpp
    src/repositories/announcement_repository.cpp
    src/repositories/read_receipt_repository.cpp
    src/cache/announcement_cache.cpp
    src/utils/json_utils.cpp
    src/utils/date_utils.cpp
    src/utils/logging.cpp
)

# 可执行文件
add_executable(announcement_server ${SOURCES})

# 链接库
target_link_libraries(announcement_server
    ${SQLite3_LIBRARIES}
    ${Boost_LIBRARIES}
)

# 数据库迁移工具
add_executable(migration_tool src/tools/migration_tool.cpp)
target_link_libraries(migration_tool ${SQLite3_LIBRARIES})

# 单元测试
set(TEST_SOURCES
    src/tests/test_announcement.cpp
    src/tests/test_read_receipt.cpp
    src/tests/test_auth.cpp
)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests ${SQLite3_LIBRARIES})

# 安装目标
install(TARGETS announcement_server migration_tool tests
    RUNTIME DESTINATION bin
)
