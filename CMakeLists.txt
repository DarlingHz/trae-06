cmake_minimum_required(VERSION 3.14)
project(recruitment_management_system CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 查找依赖
find_package(SQLite3 REQUIRED)
find_package(Threads REQUIRED)

# 包含头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)

# 收集源文件
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# 创建静态库
add_library(${PROJECT_NAME}_lib STATIC ${SOURCES})

# 链接库依赖
target_link_libraries(${PROJECT_NAME}_lib
    ${SQLite3_LIBRARIES}
    Threads::Threads
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/src/main.cpp)

# 链接可执行文件依赖
target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME}_lib
)

# 测试配置（暂时注释，等待测试文件）
# include(FetchContent)
# FetchContent_Declare(
#     Catch2
#     GIT_REPOSITORY https://github.com/catchorg/Catch2.git
#     GIT_TAG v3.4.0
# )
# FetchContent_MakeAvailable(Catch2)

# 收集测试文件
# file(GLOB_RECURSE TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/*.cpp)

# 创建测试可执行文件
# add_executable(tests ${TEST_SOURCES})

# 链接测试依赖
# target_link_libraries(tests
#     ${PROJECT_NAME}_lib
#     Catch2::Catch2WithMain
# )

# 启用测试
# enable_testing()
# add_test(NAME AllTests COMMAND tests)

# 安装配置
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES ${PROJECT_SOURCE_DIR}/db/schema.sql DESTINATION share/${PROJECT_NAME})

# 生成文档（可选）
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${PROJECT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${PROJECT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()