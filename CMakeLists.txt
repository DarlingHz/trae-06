cmake_minimum_required(VERSION 3.10)
project(survey_server VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/third_party/cpp-httplib
    ${PROJECT_SOURCE_DIR}/third_party/nlohmann
    ${PROJECT_SOURCE_DIR}/third_party/sqlite3
)

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/controller/survey_controller.cpp
    src/service/survey_service.cpp
    src/repository/survey_repository.cpp
    src/db/database.cpp
    third_party/sqlite3/sqlite3.c
)

# Add executable target
add_executable(survey_server ${SOURCE_FILES})

# Link libraries
target_link_libraries(survey_server pthread dl)

# Copy data directory to build directory
add_custom_command(TARGET survey_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${PROJECT_SOURCE_DIR}/data
    ${PROJECT_BINARY_DIR}/data
)
