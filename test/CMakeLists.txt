# HTTP服务器测试模块

# 测试源文件
set(HTTP_SERVER_TEST_SOURCES
    test_http_server.cpp
)

# 生成测试可执行文件
add_executable(test_http_server ${HTTP_SERVER_TEST_SOURCES})

# 链接到项目库
target_link_libraries(test_http_server
    PRIVATE
    job_service_lib
)

# 包含头文件
target_include_directories(test_http_server
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
)

# 添加测试
add_test(NAME HTTP_Server_Test COMMAND test_http_server)
