cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE debugplugin_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(debugplugin SHARED ${debugplugin_src})
set_target_properties(debugplugin PROPERTIES OUTPUT_NAME "debug-plugin")

target_include_directories(debugplugin
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server/home"
    "${PROJECT_SOURCE_DIR}/Dependencies"
)
target_link_libraries(debugplugin
    home
    ${Boost_LIBRARIES}
)