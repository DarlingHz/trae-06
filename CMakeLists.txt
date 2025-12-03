cmake_minimum_required(VERSION 3.10)
project(DeviceWarrantyManagementSystem)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加源文件目录
include_directories(src)
include_directories(src/models)
include_directories(src/repositories)
include_directories(src/services)
include_directories(src/controllers)
include_directories(src/utils)
include_directories(src/db)

# 收集所有源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# 创建可执行文件
add_executable(warranty_service ${SOURCES} ${HEADERS})

# 查找依赖库
find_package(Threads REQUIRED)
find_package(MySQL REQUIRED)
find_package(hiredis REQUIRED)

# 包含头文件目录
target_include_directories(warranty_service PRIVATE
    ${MySQL_INCLUDE_DIR}
    ${HIREDIS_INCLUDE_DIR}
)

# 链接依赖库
target_link_libraries(warranty_service
    Threads::Threads
    ${MySQL_LIBRARY}
    ${HIREDIS_LIBRARY}
)

# 编译选项
if(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(warranty_service PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Werror
        -O2
    )
elseif(MSVC)
    target_compile_options(warranty_service PRIVATE
        /W4
        /WX
        /O2
    )
endif()

# 测试目标
add_executable(test_warranty_service test/test_main.cpp
    src/utils/date_utils.cpp
    src/utils/logger.cpp
    src/utils/json_utils.cpp
    src/utils/config.cpp
)

target_link_libraries(test_warranty_service
    ${CMAKE_THREAD_LIBS_INIT}
    ${MySQL_LIBRARY}
    ${HIREDIS_LIBRARY}
)

install(TARGETS test_warranty_service
    RUNTIME DESTINATION bin
)

# 安装目标
install(TARGETS warranty_service
    DESTINATION /usr/local/bin
)
