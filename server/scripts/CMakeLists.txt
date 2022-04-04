cmake_minimum_required(VERSION 3.8)

file(GLOB scripts "*")
foreach(script ${scripts})
    if(IS_DIRECTORY ${script})
        message(STATUS "Found " ${script})
        add_subdirectory(${script})
    endif()
endforeach()