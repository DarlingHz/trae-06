# 收集库的源文件（不包含main.cpp）
file(GLOB_RECURSE LIBRARY_SOURCES
    "controller/*.cpp"
    "service/*.cpp"
    "repository/*.cpp"
    "model/*.cpp"
    "utils/*.cpp"
)

# 收集可执行文件的源文件（只包含main.cpp）
set(EXECUTABLE_SOURCES "main.cpp")

# 创建静态库
add_library(leaderboard_service STATIC ${LIBRARY_SOURCES})

# 创建可执行文件
add_executable(leaderboard_backend ${EXECUTABLE_SOURCES})

# 链接依赖库
target_link_libraries(leaderboard_service
    PRIVATE
        httplib::httplib
        nlohmann_json::nlohmann_json
        sqlite3
        spdlog::spdlog
        Threads::Threads
)

target_link_libraries(leaderboard_backend
    PRIVATE
        leaderboard_service
        spdlog::spdlog
)

# 包含头文件目录
target_include_directories(leaderboard_service
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(leaderboard_backend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
