cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE debug_script_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(debug_script SHARED ${debug_script_src})
set_target_properties(debug_script PROPERTIES OUTPUT_NAME "debug-script")

target_include_directories(debug_script
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server"
    "${PROJECT_SOURCE_DIR}/Dependencies"
)
target_link_libraries(debug_script
    home_common
    home_scripting
    home_scripting_native
    ${Boost_LIBRARIES}
)