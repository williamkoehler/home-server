cmake_minimum_required(VERSION 3.8)

project("Home")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fstandalone-debug")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

add_compile_definitions(OS_LINUX)
add_compile_options(-Wall -Wextra -Wpedantic)

# Conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Threads REQUIRED)

# Only compiles on x64 platforms
add_compile_definitions(SYS_X64)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/bin")

set(ENABLE_TESTS 0)

add_subdirectory("server")