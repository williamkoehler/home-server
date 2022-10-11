cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_scripting_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_scripting SHARED ${home_scripting_src})
set_target_properties(home_scripting PROPERTIES OUTPUT_NAME "home.scripting")

target_compile_definitions(home_scripting
    PUBLIC
)
target_include_directories(home_scripting
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
)
target_link_libraries(home_scripting
    home_common
    home_database
    ${CONAN_LIBS}
)