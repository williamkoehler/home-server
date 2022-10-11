cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_networking_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_networking SHARED ${home_networking_src})
set_target_properties(home_networking PROPERTIES OUTPUT_NAME "home.networking")

target_compile_definitions(home_networking
    PUBLIC
)
target_include_directories(home_networking
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
)
target_link_libraries(home_networking
    home_common
    home_database
    home_scripting
    home_main
    home_users
    ${CONAN_LIBS}
)