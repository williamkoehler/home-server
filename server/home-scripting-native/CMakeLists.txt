cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_scripting_native_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_scripting_native SHARED ${home_scripting_native_src})
set_target_properties(home_scripting_native PROPERTIES OUTPUT_NAME "home.scripting.native")

target_compile_definitions(home_scripting_native
    PUBLIC
)
target_include_directories(home_scripting_native
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server"
)
target_link_libraries(home_scripting_native
    home_common
    home_database
    home_scripting
    ${CONAN_LIBS}
)

target_precompile_headers(home_scripting_native PRIVATE "common.hpp")