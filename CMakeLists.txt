cmake_minimum_required(VERSION 3.16)
project(contract_approval_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/http
    ${CMAKE_SOURCE_DIR}/domain
    ${CMAKE_SOURCE_DIR}/storage
    ${CMAKE_SOURCE_DIR}/service
    ${CMAKE_SOURCE_DIR}/tests
)

# Source files
set(SOURCES
    main.cpp
    http/server.cpp
    storage/sqlite_storage.cpp
    service/contract_service.cpp
    service/approval_service.cpp
    domain/user.cpp
    domain/contract.cpp
    domain/approval_step.cpp
    domain/approval_log.cpp
)

# Find SQLite
find_package(SQLite3 REQUIRED)

# Find cpp-httplib
find_package(httplib CONFIG REQUIRED)

# Find nlohmann-json
find_package(nlohmann_json CONFIG REQUIRED)

# Find fmt
find_package(fmt CONFIG REQUIRED)

# Find Catch2 for testing
find_package(Catch2 CONFIG REQUIRED)

# Build main executable
add_executable(contract_server ${SOURCES})

target_link_libraries(contract_server
    PRIVATE
    SQLite::SQLite3
    httplib::httplib
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# Build tests
set(TEST_SOURCES
    tests/main_test.cpp
    tests/contract_service_test.cpp
    tests/storage_test.cpp
    http/server.cpp
    storage/sqlite_storage.cpp
    service/contract_service.cpp
    service/approval_service.cpp
    domain/user.cpp
    domain/contract.cpp
    domain/approval_step.cpp
    domain/approval_log.cpp
)

add_executable(contract_tests ${TEST_SOURCES})

target_link_libraries(contract_tests
    PRIVATE
    SQLite::SQLite3
    httplib::httplib
    nlohmann_json::nlohmann_json
    fmt::fmt
    Catch2::Catch2
    Catch2::Catch2WithMain
)

# Install target
install(TARGETS contract_server contract_service_test
    RUNTIME DESTINATION bin
)
