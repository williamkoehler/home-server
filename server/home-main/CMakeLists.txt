cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_main_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_main SHARED ${home_main_src})
set_target_properties(home_main PROPERTIES OUTPUT_NAME "home.main")

target_compile_definitions(home_main
    PUBLIC
)
target_include_directories(home_main
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(home_main
    home_common
    home_database
    home_scripting
    ${Boost_LIBRARIES}
)