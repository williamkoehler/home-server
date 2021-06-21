cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE homeserver_src "*.cpp" "*.h")

add_executable(HOMESERVER ${homeserver_src})
set_target_properties(HOMESERVER PROPERTIES OUTPUT_NAME "homeserver")

target_include_directories(HOMESERVER
    PUBLIC
    "${PROJECT_SOURCE_DIR}/Server/Home"
)

if (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
target_include_directories(HOMESERVER
    PUBLIC
    "${PROJECT_SOURCE_DIR}/Dependencies"
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(HOMESERVER optimized
    HOME
    XXHASH
    LIBQUICKMAIL
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES}
)
endif (UNIX)