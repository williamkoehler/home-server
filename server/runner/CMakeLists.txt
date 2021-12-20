cmake_minimum_required(VERSION 3.8)

add_executable(RUNNER main.cpp)
set_target_properties(RUNNER PROPERTIES OUTPUT_NAME "homeserver")

target_include_directories(RUNNER
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server/homeserver"
)
target_link_libraries(RUNNER
    HOMESERVER
)