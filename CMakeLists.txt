cmake_minimum_required(VERSION 3.16)
project(bookmark_manager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")

# 查找依赖
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)

# 查找cpp-httplib
find_path(HTTPLIB_INCLUDE_DIRS httplib.h)
if(NOT HTTPLIB_INCLUDE_DIRS)
    # 如果系统库查找失败，尝试使用brew安装的路径
    find_path(HTTPLIB_INCLUDE_DIRS httplib.h PATHS /opt/homebrew/opt/cpp-httplib/include)
endif()
if(NOT HTTPLIB_INCLUDE_DIRS)
    message(FATAL_ERROR "Could not find cpp-httplib header files")
endif()

# 查找nlohmann_json
find_package(nlohmann_json 3.10.0 REQUIRED)

# 查找fmt库
find_package(fmt REQUIRED)

# 查找Google Test库
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${HTTPLIB_INCLUDE_DIRS})

# 收集源文件
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")

# 构建可执行文件
add_executable(bookmark_service ${SOURCES})

# 链接库
target_link_libraries(bookmark_service
    ${SQLite3_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${HTTPLIB_LIBRARIES}
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# 测试可执行文件
file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.cpp")
# 排除main.cpp文件
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
add_executable(bookmark_tests ${TEST_SOURCES} ${SOURCES})

# 测试目标链接库
target_link_libraries(bookmark_tests
    ${SQLite3_LIBRARY}
    ${OPENSSL_LIBRARIES}
    ${HTTPLIB_LIBRARIES}
    nlohmann_json::nlohmann_json
    fmt::fmt
    GTest::GTest
    GTest::Main
)

# 安装目标
install(TARGETS bookmark_service DESTINATION bin)
install(FILES config/config.json.example DESTINATION share/bookmark_manager)
