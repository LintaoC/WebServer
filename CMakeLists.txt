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
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Library for handling parsing configuration files
add_library(config_parser src/config_parser.cc)
add_library(echo_handler src/EchoHandler.cc)
target_link_libraries(echo_handler Boost::system)
# Library for core server functionality
add_library(server_core
    src/server.cc
    src/session.cc
    src/EchoHandler.cc
    src/StaticFileHandler.cc
)
target_link_libraries(server_core Boost::system Boost::log Boost::log_setup)
#target_link_libraries(server_core echo_handler)

# Main server executable
add_executable(server src/server_main.cc)
target_link_libraries(server server_core config_parser Boost::log_setup Boost::log)

# Tests
# Echo Handler Test
add_executable(EchoHandler_test tests/EchoHandler_test.cc)
target_link_libraries(EchoHandler_test server_core gtest_main gtest gmock Boost::system)
gtest_discover_tests(EchoHandler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Test for the config_parser
add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser gtest_main gtest gmock Boost::log Boost::log_setup)
gtest_discover_tests(config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Server tests
add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server_core  gtest_main gtest gmock Boost::log Boost::log_setup)
gtest_discover_tests(server_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Session tests
add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test server_core  gtest_main gtest gmock Boost::log Boost::log_setup)
gtest_discover_tests(session_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Coverage configuration
include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS server_core config_parser TESTS config_parser_test server_test session_test EchoHandler_test)

add_test(NAME integration_test COMMAND bash ../tests/integration_tests.sh)