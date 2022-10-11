cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_common_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_common SHARED ${home_common_src})
set_target_properties(home_common PROPERTIES OUTPUT_NAME "home.common")

target_compile_definitions(home_common
    PUBLIC
)
target_include_directories(home_common
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
)
target_link_libraries(home_common
    ${CONAN_LIBS}
)

target_precompile_headers(home_common PRIVATE "common.hpp")