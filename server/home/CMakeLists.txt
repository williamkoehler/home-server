cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(HOME SHARED ${home_src})
set_target_properties(HOME PROPERTIES OUTPUT_NAME "home")

target_compile_definitions(HOME
    PUBLIC
)
target_include_directories(HOME
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
)
target_link_libraries(HOME
    XXHASH
    ${Boost_LIBRARIES}
)