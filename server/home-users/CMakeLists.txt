cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_users_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_users SHARED ${home_users_src})
set_target_properties(home_users PROPERTIES OUTPUT_NAME "home.users")

target_compile_definitions(home_users
    PUBLIC
)
target_include_directories(home_users
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
)
target_link_libraries(home_users
    home_common
    home_database
    ${CONAN_LIBS}
)