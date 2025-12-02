cmake_minimum_required(VERSION 3.10)

# 项目基本信息
project(LibraryManagementSystem VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 定义项目根目录
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 定义源代码目录和头文件目录
set(SRC_DIR ${ROOT_DIR}/src)
set(INCLUDE_DIR ${ROOT_DIR}/src)
set(TEST_DIR ${ROOT_DIR}/test)

# 查找所有源文件
file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS ${INCLUDE_DIR}/*.h)

# 定义可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 设置头文件搜索路径
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

# 查找并链接外部库

# 设置CMP0167策略以解决FindBoost模块被移除的警告
cmake_policy(SET CMP0167 NEW)

# Boost库
find_package(Boost REQUIRED COMPONENTS thread date_time chrono regex filesystem)
if(Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_THREAD_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
endif()

# MySQL Connector/C++库
set(MYSQLCONNECTORCPP_INCLUDE_DIR /opt/homebrew/Cellar/mysql-connector-c++/9.5.0/include)
set(MYSQLCONNECTORCPP_LIBRARY /opt/homebrew/lib/libmysqlcppconnx.dylib)
if(EXISTS ${MYSQLCONNECTORCPP_INCLUDE_DIR} AND EXISTS ${MYSQLCONNECTORCPP_LIBRARY})
    target_include_directories(${PROJECT_NAME} PRIVATE ${MYSQLCONNECTORCPP_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${MYSQLCONNECTORCPP_LIBRARY})
else()
    message(FATAL_ERROR "MySQL Connector/C++ library not found at ${MYSQLCONNECTORCPP_INCLUDE_DIR} or ${MYSQLCONNECTORCPP_LIBRARY}")
endif()

# libtomcrypt库
find_library(LIBTOMCRYPT_LIBRARY NAMES tomcrypt)
if(LIBTOMCRYPT_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBTOMCRYPT_LIBRARY})
else()
    message(FATAL_ERROR "libtomcrypt library not found")
endif()

# libjwt库
find_library(LIBJWT_LIBRARY NAMES jwt)
if(LIBJWT_LIBRARY)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBJWT_LIBRARY})
else()
    message(FATAL_ERROR "libjwt library not found")
endif()

# jwt-cpp库
set(JWT_CPP_INCLUDE_DIR /Users/soma/code/trae/12-02/02/06/jwt-cpp/include)
if(EXISTS ${JWT_CPP_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} PRIVATE ${JWT_CPP_INCLUDE_DIR})
else()
    message(FATAL_ERROR "jwt-cpp library not found at ${JWT_CPP_INCLUDE_DIR}")
endif()

# bcrypt库 - 已使用SHA-256替代，不再需要

# OpenSSL库（jwt-cpp依赖）
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${OpenSSL_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenSSL_LIBRARIES})
else()
    message(FATAL_ERROR "OpenSSL library not found")
endif()

# 手动添加OpenSSL库的链接
if(APPLE)
    # macOS平台
    target_link_libraries(${PROJECT_NAME} PRIVATE /opt/homebrew/lib/libssl.dylib /opt/homebrew/lib/libcrypto.dylib)
elseif(UNIX)
    # Linux平台
    target_link_libraries(${PROJECT_NAME} PRIVATE ssl crypto)
endif()

# cpprestsdk库
find_package(cpprestsdk REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE cpprestsdk::cpprest)

# 设置编译选项
if(MSVC)
    # Windows平台编译选项
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    # Linux/macOS平台编译选项
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# 设置安装规则
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})

# 启用测试（可选）
enable_testing()
if(BUILD_TESTING)
    # 查找所有测试文件
    file(GLOB_RECURSE TEST_SOURCES ${TEST_DIR}/*.cpp)
    
    # 定义测试可执行文件
    add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES} ${SOURCES} ${HEADERS})
    
    # 设置测试头文件搜索路径
    target_include_directories(${PROJECT_NAME}_tests PRIVATE ${INCLUDE_DIR} ${TEST_DIR})
    
    # 链接测试所需的库
    target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${Boost_LIBRARIES} ${MySQLConnectorCPP_LIBRARIES} ${BCRYPT_LIBRARY} ${jwtcpp_LIBRARIES} ${OpenSSL_LIBRARIES})
    
    # 添加测试
    add_test(NAME ${PROJECT_NAME}_tests COMMAND ${PROJECT_NAME}_tests)
endif()

# 生成项目文档（可选）
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${ROOT_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    # 配置Doxyfile
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    # 生成文档
    add_custom_target(doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()