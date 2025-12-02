cmake_minimum_required(VERSION 3.15)
project(meal_plan_manager VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/crow/include
    ${PROJECT_SOURCE_DIR}/third_party/nlohmann_json/include
)

# 查找SQLite3
find_package(SQLite3 REQUIRED)
if(SQLite3_FOUND)
    include_directories(${SQLite3_INCLUDE_DIRS})
    link_libraries(${SQLite3_LIBRARIES})
endif()

# 源文件
set(SOURCE_FILES
    src/main.cpp
    src/database.cpp
    src/user_service.cpp
    src/recipe_service.cpp
    src/meal_plan_service.cpp
    src/shopping_list_service.cpp
)

# 创建可执行文件
add_executable(meal_plan_manager ${SOURCE_FILES})

# 链接库
target_link_libraries(meal_plan_manager
    ${SQLite3_LIBRARIES}
    pthread
)

# 安装目标
install(TARGETS meal_plan_manager DESTINATION bin)
install(DIRECTORY config DESTINATION etc/meal_plan_manager)
install(DIRECTORY DESTINATION var/lib/meal_plan_manager/data)

# 测试配置
enable_testing()
find_package(Catch2 REQUIRED)

# 测试源文件
set(TEST_SOURCE_FILES
    tests/shopping_list_test.cpp
    src/database.cpp
    src/shopping_list_service.cpp
)

# 创建测试可执行文件
add_executable(shopping_list_test ${TEST_SOURCE_FILES})

# 链接测试库
target_link_libraries(shopping_list_test
    ${SQLite3_LIBRARIES}
    Catch2::Catch2
    pthread
)

# 添加测试
add_test(NAME ShoppingListTest COMMAND shopping_list_test)

# 构建类型配置
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 编译选项
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(meal_plan_manager PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(shopping_list_test PRIVATE -Wall -Wextra -pedantic)
endif()

# 确保数据目录存在
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/data)

# 复制配置文件到构建目录
configure_file(${PROJECT_SOURCE_DIR}/config/config.json ${PROJECT_BINARY_DIR}/config/config.json COPYONLY)
