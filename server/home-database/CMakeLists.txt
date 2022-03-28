cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE home_database_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(home_database SHARED ${home_database_src})
set_target_properties(home_database PROPERTIES OUTPUT_NAME "home.database")

target_include_directories(home_database
    PUBLIC
    "${PROJECT_SOURCE_DIR}/dependencies"
    "${PROJECT_SOURCE_DIR}/server"
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)
target_link_libraries(home_database
    home_common
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
)