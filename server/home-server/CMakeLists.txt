cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_server_src "*.cpp" "*.hpp" "*.c" "*.h")

add_executable(home_server ${home_server_src})
set_target_properties(home_server PROPERTIES OUTPUT_NAME "home-server")

target_include_directories(home_server
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
)
target_link_libraries(home_server
    home_common
    home_database
    home_scripting
    home_scripting_native
    home_scripting_javascript
    home_users
    home_networking
    dl
    ${CONAN_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)