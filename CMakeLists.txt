cmake_minimum_required(VERSION 3.14)
project(tech_qa_backend)

# C++标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# 包含目录
include_directories(include)
include_directories(src)

# 查找SQLite3
find_package(SQLite3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIRS})

# 源文件
set(SOURCE_FILES
    src/main.cpp
    src/http_server.cpp
    src/controllers/user_controller.cpp
    src/services/user_service.cpp
    src/repositories/user_repository.cpp
    src/models/user.cpp
    src/models/question.cpp
    src/models/answer.cpp
    src/models/tag.cpp
    src/models/like.cpp
    src/utils/config.cpp
    src/utils/json.cpp
    src/utils/jwt.cpp
    src/utils/hash.cpp
    src/utils/logger.cpp
)

# 可执行文件
add_executable(tech_qa_backend ${SOURCE_FILES})

# 链接库
target_link_libraries(tech_qa_backend ${SQLITE3_LIBRARIES})

# 测试程序（暂时注释，待测试文件创建后再启用）
# add_executable(run_tests
#     tests/main_test.cpp
#     tests/user_service_test.cpp
#     tests/rate_limiter_test.cpp
#     tests/lru_cache_test.cpp
#     src/services/user_service.cpp
#     src/repositories/user_repository.cpp
#     src/utils/config.cpp
#     src/utils/json.cpp
#     src/utils/jwt.cpp
#     src/utils/hash.cpp
#     src/utils/logger.cpp
#     src/utils/rate_limiter.cpp
#     src/utils/lru_cache.cpp
#     src/utils/db_connection_pool.cpp
# )
# 
# target_link_libraries(run_tests ${SQLITE3_LIBRARIES})

# 安装目标
install(TARGETS tech_qa_backend DESTINATION bin)
install(FILES config/config.json DESTINATION config)
