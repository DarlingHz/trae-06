cmake_minimum_required(VERSION 3.14)
project(giftcard_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/model)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/service)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/repository)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/controller)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/utils)
include_directories(/opt/homebrew/opt/hiredis/include)

# 自动生成 Drogon 控制器和模型
file(GLOB_RECURSE CONTROLLER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/controller/*.cc)
file(GLOB_RECURSE MODEL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/model/*.cc)
file(GLOB_RECURSE SERVICE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/service/*.cc)
file(GLOB_RECURSE REPOSITORY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/repository/*.cc)
file(GLOB_RECURSE UTILS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cc)

# 生成可执行文件
add_executable(giftcard_server main.cc 
    ${CONTROLLER_SOURCES} 
    ${MODEL_SOURCES} 
    ${SERVICE_SOURCES} 
    ${REPOSITORY_SOURCES} 
    ${UTILS_SOURCES})

# 链接依赖库
target_link_libraries(giftcard_server PRIVATE Drogon::Drogon)
target_link_libraries(giftcard_server PRIVATE hiredis)
target_link_libraries(giftcard_server PRIVATE nlohmann_json::nlohmann_json)

# 复制配置文件示例到构建目录
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.example.yaml ${CMAKE_CURRENT_BINARY_DIR}/config.yaml COPYONLY)

# 安装规则
install(TARGETS giftcard_server DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.example.yaml DESTINATION etc/giftcard_server RENAME config.yaml)

# 添加测试
# enable_testing()
# add_subdirectory(tests)
