cmake_minimum_required(VERSION 3.15)
project(trae-06 VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compile options
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Include FetchContent module
include(FetchContent)

# Fetch asio library
FetchContent_Declare(
    asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-28-0
)
FetchContent_MakeAvailable(asio)

# Get asio library properties
FetchContent_GetProperties(asio)

# Set ASIO_INCLUDE_DIR variable to use the fetched asio library
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE PATH "asio include directory")

# Print asio library properties for debugging
message(STATUS "asio_SOURCE_DIR: ${asio_SOURCE_DIR}")
message(STATUS "asio_BINARY_DIR: ${asio_BINARY_DIR}")
message(STATUS "ASIO_INCLUDE_DIR: ${ASIO_INCLUDE_DIR}")

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${asio_SOURCE_DIR}/asio/include
)

# Set ASIO_INCLUDE_DIR variable to use the fetched asio library
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE PATH "asio include directory" FORCE)

# Fetch Crow library
FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.1.0
)
# Set Crow options to use external asio
set(CROW_USE_EXTERNAL_ASIO ON CACHE BOOL "Use external asio library")
FetchContent_MakeAvailable(Crow)

# Try to find nlohmann_json library in the system
find_package(nlohmann_json QUIET)

# If nlohmann_json library is not found, fetch it from GitHub
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

# Add subdirectories
add_subdirectory(src)

# Install target
install(TARGETS trae-06 DESTINATION bin)
