cmake_minimum_required(VERSION 3.8)

add_executable(runner main.cpp)
set_target_properties(runner PROPERTIES OUTPUT_NAME "homeserver")

target_include_directories(runner
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server/homeserver"
)
target_link_libraries(runner
    homeserver
)