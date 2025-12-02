cmake_minimum_required(VERSION 3.10)
project(GymBookingSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(oatpp REQUIRED)
find_package(oatpp-sqlite REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/packages/oatpp_arm64-osx/include/oatpp-1.3.0
    ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/packages/oatpp-sqlite_arm64-osx/include/oatpp-sqlite-1.3.0
    ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/packages/sqlite3_arm64-osx/include
    ${SQLite3_INCLUDE_DIRS}
)

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/service/MemberService.cpp
    src/service/CoachService.cpp
    src/service/ClassTemplateService.cpp
    src/service/ClassSessionService.cpp
    src/service/BookingService.cpp
    src/service/TrainingLogService.cpp
    src/service/StatsService.cpp
    src/data/MemberDao.cpp
    src/data/CoachDao.cpp
    src/data/ClassTemplateDao.cpp
    src/data/ClassSessionDao.cpp
    src/data/BookingDao.cpp
    src/data/TrainingLogDao.cpp
    src/data/Database.cpp
    src/cache/SessionCache.cpp
    src/util/Config.cpp
    src/util/Logger.cpp
)

# Header files
set(HEADER_FILES
    src/controller/MemberController.hpp
    src/controller/CoachController.hpp
    src/controller/ClassTemplateController.hpp
    src/controller/ClassSessionController.hpp
    src/controller/BookingController.hpp
    src/controller/TrainingLogController.hpp
    src/controller/StatsController.hpp
    src/service/MemberService.hpp
    src/service/CoachService.hpp
    src/service/ClassTemplateService.hpp
    src/service/ClassSessionService.hpp
    src/service/BookingService.hpp
    src/service/TrainingLogService.hpp
    src/service/StatsService.hpp
    src/data/MemberDao.hpp
    src/data/CoachDao.hpp
    src/data/ClassTemplateDao.hpp
    src/data/ClassSessionDao.hpp
    src/data/BookingDao.hpp
    src/data/TrainingLogDao.hpp
    src/data/Database.hpp
    src/cache/SessionCache.hpp
    src/util/Config.hpp
    src/util/Logger.hpp
    src/dto/DTOs.hpp
)

# Add executable
add_executable(gym_booking_system ${SOURCE_FILES} ${HEADER_FILES})

# Link libraries
target_link_libraries(gym_booking_system
    oatpp::oatpp
    oatpp::oatpp-sqlite
    nlohmann_json::nlohmann_json
    ${SQLite3_LIBRARIES}
)

# Copy config file to build directory
configure_file(config.json config.json COPYONLY)

# Install target
install(TARGETS gym_booking_system DESTINATION bin)
install(FILES config.json DESTINATION bin)
