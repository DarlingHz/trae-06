cmake_minimum_required(VERSION 3.14)

# 寻找Google Test库
find_package(GTest REQUIRED)

# 包含头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${GTEST_INCLUDE_DIRS}
)

# 收集测试源文件
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# 创建测试可执行文件
add_executable(chat_archive_test ${TEST_SOURCES})

# 链接库
target_link_libraries(chat_archive_test
    chat_archive_core
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

# 添加测试
gtest_discover_tests(chat_archive_test)

# 安装测试可执行文件（可选）
install(TARGETS chat_archive_test DESTINATION bin)