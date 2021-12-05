cmake_minimum_required(VERSION 3.8)

file(GLOB_RECURSE homeserver_src "*.cpp" "*.hpp" "*.c" "*.h")

macro(homeserver_definition proj)
    target_include_directories(${proj}
        PUBLIC
        "${PROJECT_SOURCE_DIR}/server/home"
        "${PROJECT_SOURCE_DIR}/dependencies"
        ${OPENSSL_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS}
    )
    target_link_libraries(${proj}
        HOME
        XXHASH
        LIBQUICKMAIL
        dl
        ${CMAKE_THREAD_LIBS_INIT}
        ${OPENSSL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${SQLite3_LIBRARIES}
    )
endmacro()

add_executable(HOMESERVER ${homeserver_src})
set_target_properties(HOMESERVER PROPERTIES OUTPUT_NAME "homeserver")
homeserver_definition(HOMESERVER)

if(${ENABLE_TESTS})
    add_library(LIBHOMESERVER ${homeserver_src})
    homeserver_definition(LIBHOMESERVER)
endif()