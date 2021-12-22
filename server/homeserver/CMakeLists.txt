cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE homeserver_src "*.cpp" "*.hpp" "*.c" "*.h")

add_library(homeserver SHARED ${homeserver_src})
set_target_properties(homeserver PROPERTIES OUTPUT_NAME "homeserver")

target_include_directories(homeserver
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server/home"
    "${PROJECT_SOURCE_DIR}/dependencies"
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)
target_link_libraries(homeserver
    home
    xxhash
    libquickmail
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
)