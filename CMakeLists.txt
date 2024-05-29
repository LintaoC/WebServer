cmake_minimum_required(VERSION 3.10)
project(f004)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory and run CMake from there.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include(CTest)
enable_testing()

# Assuming GoogleTest and Boost are configured correctly
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log log_setup)
find_package(Boost REQUIRED)  # Add this to find the general Boost package
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)

# EntityDatabase library
add_library(EntityDatabase src/EntityDatabase.cc)
target_include_directories(EntityDatabase PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EntityDatabase Boost::system)  

# Library for handling parsing configuration files
add_library(config_parser src/config_parser.cc src/RequestHandlerFactory.cc src/EchoHandler.cc src/StaticFileHandler.cc src/NotFoundHandler.cc src/CRUDHandler.cc src/SleepRequestHandler.cc src/HealthHandler.cc)
add_library(echo_handler src/EchoHandler.cc)
add_library(static_handler src/StaticFileHandler.cc)
add_library(not_found_handler src/NotFoundHandler.cc)
add_library(crud_handler src/CRUDHandler.cc)
add_library(health_handler src/HealthHandler.cc)
add_library(handler_factory src/RequestHandlerFactory.cc src/EchoHandler.cc src/StaticFileHandler.cc src/NotFoundHandler.cc src/CRUDHandler.cc  src/SleepRequestHandler.cc src/HealthHandler.cc)
target_link_libraries(echo_handler Boost::system)
# Library for core server functionality
add_library(server_core
        src/server.cc
        src/session.cc
        src/EchoHandler.cc
        src/StaticFileHandler.cc
        src/RequestHandlerFactory.cc
        src/NotFoundHandler.cc
        src/CRUDHandler.cc
        src/EntityDatabase.cc
        src/SleepRequestHandler.cc
        src/HealthHandler.cc
)
target_link_libraries(server_core Boost::system Boost::log Boost::log_setup)
#target_link_libraries(server_core echo_handler)

# Main server executable
add_executable(server src/server_main.cc)
target_link_libraries(server server_core config_parser Boost::log_setup Boost::log)

# --- UNIT TEST ---
# Entity Database Test
add_executable(EntityDatabaseTest tests/EntityDatabaseTest.cc)
target_link_libraries(EntityDatabaseTest EntityDatabase gtest_main gtest gmock Boost::system)
gtest_discover_tests(EntityDatabaseTest WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Handler Factory Test
add_executable(RequestHandlerFactoryTest tests/RequestHandlerFactoryTest.cc)
target_link_libraries(RequestHandlerFactoryTest server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(RequestHandlerFactoryTest WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Echo Handler Test
add_executable(EchoHandler_test tests/EchoHandler_test.cc)
target_link_libraries(EchoHandler_test server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(EchoHandler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Health Handler Test
add_executable(HealthHandler_test tests/HealthHandler_test.cc)
target_link_libraries(HealthHandler_test server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(HealthHandler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Sleep Handler Test
add_executable(SleepRequestHandler_test tests/SleepRequestHandler_test.cc)
target_link_libraries(SleepRequestHandler_test server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(SleepRequestHandler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Not Found Handler Test
add_executable(NotFoundHandler_test tests/NotFoundHandlerTest.cc)
target_link_libraries(NotFoundHandler_test server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(NotFoundHandler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Static Handler Test
add_executable(StaticFileHandlerTest tests/static_file_handler_test.cc)
target_link_libraries(StaticFileHandlerTest server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(StaticFileHandlerTest WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# CRUD Handler Test
add_executable(CRUDHandler_test tests/CRUDHandler_test.cc)
target_link_libraries(CRUDHandler_test server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(CRUDHandler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# config_parser tests
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser EntityDatabase gtest_main gtest gmock Boost::log Boost::log_setup)
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Server tests
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_core  config_parser gtest_main gtest gmock Boost::log Boost::log_setup)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Session tests
add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test server_core  config_parser gtest_main gtest gmock Boost::log Boost::log_setup)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Coverage configuration
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS server_core config_parser EntityDatabase TESTS config_parser_test server_test session_test EntityDatabaseTest CRUDHandler_test EchoHandler_test StaticFileHandlerTest RequestHandlerFactoryTest NotFoundHandler_test HealthHandler_test SleepRequestHandler_test)

# --- INTEGRATION TEST ---
add_test(NAME integration_test COMMAND bash ../integration_tests/integration_tests.sh)