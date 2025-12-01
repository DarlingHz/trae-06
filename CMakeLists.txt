cmake_minimum_required(VERSION 3.14)
project(chat_archive_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 寻找第三方库
find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)

# 手动查找spdlog库
find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h)
find_library(SPDLOG_LIBRARY NAMES spdlog spdlogd)

if(NOT SPDLOG_INCLUDE_DIR)
    message(FATAL_ERROR "spdlog header files not found")
endif()

if(NOT SPDLOG_LIBRARY)
    message(FATAL_ERROR "spdlog library not found")
endif()

# 包含头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    ${SPDLOG_INCLUDE_DIR}
    ${fmt_INCLUDE_DIRS}
)

# 收集源文件（不包括main.cpp）
file(GLOB_RECURSE CORE_SOURCES ${PROJECT_SOURCE_DIR}/src/chat_archive/*.cpp)
file(GLOB_RECURSE MAIN_SOURCE ${PROJECT_SOURCE_DIR}/src/main.cpp)

# 创建核心库文件
add_library(chat_archive_core STATIC ${CORE_SOURCES})

# 链接核心库的依赖
target_link_libraries(chat_archive_core
    ${SQLite3_LIBRARIES}
    ${SPDLOG_LIBRARY}
    fmt::fmt
)

# 创建可执行文件
add_executable(chat_archive_server ${MAIN_SOURCE})

# 链接可执行文件的依赖
target_link_libraries(chat_archive_server
    chat_archive_core
    fmt::fmt
)

# 安装目标
install(TARGETS chat_archive_server chat_archive_core DESTINATION bin)

# 安装目标
install(TARGETS chat_archive_server DESTINATION bin)

# 启用测试
# enable_testing()
# add_subdirectory(tests)