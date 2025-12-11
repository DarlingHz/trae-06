# 测试项目的CMakeLists.txt文件

# 查找Catch2测试框架
find_package(Catch2 REQUIRED)

# 添加测试目标
add_executable(unit_tests
    unit/leaderboard_test.cpp
)

# 链接测试目标到需要的库
target_link_libraries(unit_tests
    PRIVATE
    leaderboard_service
    Catch2::Catch2
)

# 包含测试目标的头文件目录
target_include_directories(unit_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 注册测试
include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
