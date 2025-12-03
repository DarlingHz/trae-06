cmake_minimum_required(VERSION 3.14)
project(bike-sharing-backend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找依赖
find_package(Threads REQUIRED)

# 设置头文件目录
include_directories(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
)

# 收集源文件
file(GLOB_RECURSE SOURCES
  ${CMAKE_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/include/*.h
  ${CMAKE_SOURCE_DIR}/*.cpp
  ${CMAKE_SOURCE_DIR}/*.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接依赖
target_link_libraries(${PROJECT_NAME}
  Threads::Threads
)

# 编译选项
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

# 测试目标
add_subdirectory(tests)