cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE debugplugin_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(DEBUGPLUGIN SHARED ${debugplugin_src})
set_target_properties(DEBUGPLUGIN PROPERTIES OUTPUT_NAME "debug-plugin")

target_include_directories(DEBUGPLUGIN
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server/home"
    "${PROJECT_SOURCE_DIR}/Dependencies"
)
target_link_libraries(DEBUGPLUGIN
    HOME
    ${Boost_LIBRARIES}
)