cmake_minimum_required(VERSION 3.8)

file(GLOB scripts "*")
foreach(script ${scripts})
    if(IS_DIRECTORY ${script} AND EXISTS "${script}/CMakeLists.txt")
        message(STATUS "Found ${script}")
        add_subdirectory(${script})
    endif()
endforeach()