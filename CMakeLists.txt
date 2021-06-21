cmake_minimum_required(VERSION 3.8)

project("Home")

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    message(ERROR "Windows is currently not supported in cmake")
    add_compile_definitions(OS_WINDOWS)
    # Default path

    set(BOOST_ROOT "C:\\tools\\boost_1_75_0")
    set(OPENSSL_ROOT_DIR "C:\\tools\\openssl")
    find_package(Boost REQUIRED COMPONENTS chrono filesystem system thread)
    find_package(Threads REQUIRED)
    find_package(OpenSSL REQUIRED)
endif (WIN32)

if (UNIX)
    add_compile_definitions(OS_LINUX)
    find_package(Boost 1.75 REQUIRED COMPONENTS chrono filesystem system thread)
    find_package(Threads REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(libssh REQUIRED)
    find_package(CURL REQUIRED)
endif (UNIX)

# Only compiles on x64 platforms
add_compile_definitions(SYS_X64)

if (MSVC)
    add_compile_options(/bigobj)
endif (MSVC)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/out/bin")

add_subdirectory("dependencies")
add_subdirectory("server")