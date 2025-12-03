cmake_minimum_required(VERSION 3.10)
project(lost_and_found_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(SQLite3 REQUIRED)

# 包含头文件目录
include_directories(include)
include_directories(thirdparty)

# 源文件
set(SOURCES
    src/main.cpp
    src/server.cpp
    src/config.cpp
    src/database.cpp
    src/logger.cpp
    src/auth.cpp
    src/user_service.cpp
    src/lost_item_service.cpp
    src/found_item_service.cpp
    src/claim_service.cpp
    src/notification_service.cpp
    src/dashboard_service.cpp
    src/dto.cpp
    src/json_utils.cpp
)

# 头文件
set(HEADERS
    include/config.h
    include/database.h
    include/logger.h
    include/auth.h
    include/dto.h
    include/user_service.h
    include/lost_item_service.h
    include/found_item_service.h
    include/claim_service.h
    include/notification_service.h
    include/dashboard_service.h
)

# 创建可执行文件
add_executable(lost_and_found ${SOURCES} ${HEADERS})

# 链接依赖
find_package(OpenSSL REQUIRED)
target_link_libraries(lost_and_found OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(lost_and_found SQLite::SQLite3 pthread)

# 输出目录
set_target_properties(lost_and_found PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)