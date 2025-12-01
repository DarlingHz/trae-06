cmake_minimum_required(VERSION 3.10) 
project(SleepTracker) 

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

# 查找依赖库 
find_package(SQLite3 REQUIRED) 
find_package(nlohmann_json REQUIRED) 
find_package(OpenSSL REQUIRED) 

# 包含头文件目录 
include_directories(${PROJECT_SOURCE_DIR}/include) 

# 收集所有源文件 
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp) 

# 创建可执行文件 
add_executable(sleep_tracker ${SOURCES}) 

# 链接依赖库 
target_link_libraries(sleep_tracker PRIVATE SQLite::SQLite3 nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto pthread) 

# 测试 
enable_testing() 
file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/*.cpp) 
# 从SOURCES中排除src/main.cpp
list(REMOVE_ITEM SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp) 
add_executable(test_stats ${SOURCES} ${TEST_SOURCES}) 
target_link_libraries(test_stats PRIVATE SQLite::SQLite3 nlohmann_json::nlohmann_json OpenSSL::SSL OpenSSL::Crypto pthread) 
add_test(NAME test_stats COMMAND test_stats) 
