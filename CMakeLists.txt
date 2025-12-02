cmake_minimum_required(VERSION 3.15)
project(code_snippet_hosting)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Boost libraries
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
link_directories(/opt/homebrew/opt/boost/lib)

# Find OpenSSL libraries
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Add SQLite3 library
add_library(sqlite3 STATIC third_party/sqlite3/sqlite3.c)
target_compile_definitions(sqlite3 PRIVATE SQLITE_THREADSAFE=1)

# Source files
file(GLOB_RECURSE SOURCES src/*.cpp)

# Executable
add_executable(code_snippet_server ${SOURCES})

# Include directories
target_include_directories(code_snippet_server PRIVATE include)
target_include_directories(code_snippet_server PRIVATE third_party/nlohmann)
target_include_directories(code_snippet_server PRIVATE third_party/sqlite3)

# Link libraries
target_link_libraries(code_snippet_server boost_thread)
target_link_libraries(code_snippet_server sqlite3)
target_link_libraries(code_snippet_server ${OPENSSL_LIBRARIES})
target_link_libraries(code_snippet_server pthread)
target_link_libraries(code_snippet_server dl)

# Install target
install(TARGETS code_snippet_server DESTINATION bin)

# Testing
# enable_testing()
# add_subdirectory(tests)