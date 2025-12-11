# Add test executable
add_executable(trae-06-tests
)

# Add test source files
file(GLOB_RECURSE TEST_SOURCES
    ${PROJECT_SOURCE_DIR}/tests/unit/*.cpp
    ${PROJECT_SOURCE_DIR}/tests/integration/*.cpp
)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(trae-06-tests
    OpenSSL::SSL
    OpenSSL::Crypto
    SQLite3::SQLite3
    Crow::Crow
    nlohmann_json::nlohmann_json
    pthread
)

# Add test source files to executable
target_sources(trae-06-tests PRIVATE ${TEST_SOURCES})

# Add test to CTest
add_test(NAME trae-06-tests COMMAND trae-06-tests)
