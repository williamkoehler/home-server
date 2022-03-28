cmake_minimum_required(VERSION 3.8)

add_subdirectory(toolkit)

add_subdirectory(home-common)
add_subdirectory(home-threading)
add_subdirectory(home-database)
add_subdirectory(home-users)
add_subdirectory(home-scripting)
add_subdirectory(home-scripting-native)
add_subdirectory(home-scripting-javascript)
add_subdirectory(home-main)
add_subdirectory(home-networking)

add_subdirectory(scripts)

add_subdirectory(home-server)

add_subdirectory(tests)