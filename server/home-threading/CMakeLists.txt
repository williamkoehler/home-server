cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_threading_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_threading SHARED ${home_threading_src})
set_target_properties(home_threading PROPERTIES OUTPUT_NAME "home.threading")

target_compile_definitions(home_threading
    PUBLIC
)
target_include_directories(home_threading
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(home_threading
    home_common
    ${Boost_LIBRARIES}
)