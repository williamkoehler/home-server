cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE homeserver_src "*.cpp" "*.hpp" "*.c" "*.h")

add_executable(HOMESERVER ${homeserver_src})
set_target_properties(HOMESERVER PROPERTIES OUTPUT_NAME "homeserver")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
target_include_directories(HOMESERVER
    PUBLIC
    "${PROJECT_SOURCE_DIR}/server/home"
    "${PROJECT_SOURCE_DIR}/dependencies"
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${SQLite3_INCLUDE_DIRS}
)
target_link_libraries(HOMESERVER
    HOME
    XXHASH
    LIBQUICKMAIL
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${SQLite3_LIBRARIES}
)