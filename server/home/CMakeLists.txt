cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home SHARED ${home_src})
set_target_properties(home PROPERTIES OUTPUT_NAME "home")

target_compile_definitions(home
    PUBLIC
)
target_include_directories(home
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
)
target_link_libraries(home
    xxhash
    ${Boost_LIBRARIES}
)