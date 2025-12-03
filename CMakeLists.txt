# CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 项目名称和语言
project(job_service
    VERSION 1.0.0
    DESCRIPTION "通用异步任务队列服务"
    LANGUAGES CXX
)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic -O2)
endif()

# 生成可执行文件的目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 库和头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party
)

# 源代码文件
set(SOURCE_FILES
    src/job.cpp
    src/job_queue.cpp
    src/task_executor.cpp
    src/task_factory.cpp
    src/storage.cpp
    src/http_server.cpp
    src/utils.cpp
    src/main.cpp
    src/example_tasks.cpp
)

# 创建静态库
add_library(job_service_lib STATIC
    src/job.cpp
    src/job_queue.cpp
    src/task_executor.cpp
    src/task_factory.cpp
    src/storage.cpp
    src/http_server.cpp
    src/utils.cpp
)

# 创建主可执行文件
add_executable(job_service ${SOURCE_FILES})

# 链接库
find_package(Threads REQUIRED)
target_link_libraries(job_service PRIVATE Threads::Threads)

# 测试目录
add_subdirectory(test)

# 安装目标
install(TARGETS job_service
    DESTINATION bin
)

install(TARGETS job_service_lib
    DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
)

# 打印配置信息
message(STATUS "=== Job Service Configuration ===")
message(STATUS "Project name: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Source directory: ${PROJECT_SOURCE_DIR}")
message(STATUS "Binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "=================================")
