cmake_minimum_required(VERSION 3.8)

project("Home")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fstandalone-debug")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

add_compile_definitions(OS_LINUX)
find_package(Boost 1.75 REQUIRED COMPONENTS chrono filesystem system thread regex unit_test_framework)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

# Only compiles on x64 platforms
add_compile_definitions(SYS_X64)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/bin")

set(ENABLE_TESTS 1)

add_subdirectory("dependencies")
add_subdirectory("server")