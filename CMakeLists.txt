cmake_minimum_required(VERSION 3.14)
project(pet_hospital_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 查找依赖
find_package(Boost 1.70 REQUIRED COMPONENTS thread)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json REQUIRED)

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
)

# 源文件
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# 可执行文件
add_executable(pet_hospital_server ${SOURCES})

# 链接库
target_link_libraries(pet_hospital_server
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# 安装目标
install(TARGETS pet_hospital_server DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/sql DESTINATION share/pet_hospital_server)
install(FILES ${PROJECT_SOURCE_DIR}/config.json DESTINATION etc/pet_hospital_server)

# 测试
# enable_testing()
# add_subdirectory(tests)
