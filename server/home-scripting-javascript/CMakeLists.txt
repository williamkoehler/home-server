cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_scripting_javascript_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_scripting_javascript SHARED ${home_scripting_javascript_src})
set_target_properties(home_scripting_javascript PROPERTIES OUTPUT_NAME "home.scripting.javascript")

target_compile_definitions(home_scripting_javascript
    PUBLIC
)
target_include_directories(home_scripting_javascript
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(home_scripting_javascript
    home_common
    home_threading
    home_database
    home_scripting
    xxhash
    ${Boost_LIBRARIES}
)