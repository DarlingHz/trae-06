# Add executable
add_executable(trae-06 main.cpp)

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
if(NOT SQLite3_FOUND)
    find_library(SQLite3_LIBRARY sqlite3 HINTS /opt/homebrew/lib)
    find_path(SQLite3_INCLUDE_DIR sqlite3.h HINTS /opt/homebrew/include)
    if(SQLite3_LIBRARY AND SQLite3_INCLUDE_DIR)
        set(SQLite3_FOUND TRUE)
        add_library(SQLite3::SQLite3 UNKNOWN IMPORTED)
        set_target_properties(SQLite3::SQLite3 PROPERTIES
            IMPORTED_LOCATION ${SQLite3_LIBRARY}
            INTERFACE_INCLUDE_DIRECTORIES ${SQLite3_INCLUDE_DIR}
        )
    endif()
endif()

# Add source files
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/controllers/*.cpp
    ${PROJECT_SOURCE_DIR}/src/models/*.cpp
    ${PROJECT_SOURCE_DIR}/src/daos/*.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/*.cpp
    ${PROJECT_SOURCE_DIR}/src/routes/*.cpp
)

# Add header files
file(GLOB_RECURSE HEADERS
    ${PROJECT_SOURCE_DIR}/include/controllers/*.hpp
    ${PROJECT_SOURCE_DIR}/include/models/*.hpp
    ${PROJECT_SOURCE_DIR}/include/daos/*.hpp
    ${PROJECT_SOURCE_DIR}/include/utils/*.hpp
    ${PROJECT_SOURCE_DIR}/include/routes/*.hpp
)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${asio_SOURCE_DIR}/asio/include
)

# Link libraries
target_link_libraries(trae-06
    OpenSSL::SSL
    OpenSSL::Crypto
    ${SQLite3_LIBRARIES}
    Crow::Crow
    nlohmann_json::nlohmann_json
    pthread
)

# Include SQLite3 header files
target_include_directories(trae-06 PRIVATE ${SQLite3_INCLUDE_DIRS})

# Add source files to executable
target_sources(trae-06 PRIVATE ${SOURCES} ${HEADERS} ${PROJECT_SOURCE_DIR}/src/utils/BcryptUtils.cpp ${PROJECT_SOURCE_DIR}/src/daos/TagDAO.cpp)
