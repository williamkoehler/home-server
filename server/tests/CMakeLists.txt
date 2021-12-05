if(${ENABLE_TESTS})
    message(STATUS "Using tests")

    # Find all test files
    file(GLOB_RECURSE TEST_SOURCES "Test*.cpp")

    # Add one test for each test file
    foreach( testfile ${TEST_SOURCES} )
        string(REPLACE ".cpp" "" testname ${testfile})
        string(REPLACE ${PROJECT_SOURCE_DIR} "" testname ${testname})
        message(STATUS "Found " ${testname})
        string(REGEX MATCH "Test[^.]*" exename ${testname})

        add_executable(${exename} ${testfile} "common.hpp")
    
        # Add necessary libraries
        target_include_directories(${exename}
            PUBLIC
            "${PROJECT_SOURCE_DIR}/server/home"
            "${PROJECT_SOURCE_DIR}/server/homeserver"
            "${PROJECT_SOURCE_DIR}/server/plugins"
            "${PROJECT_SOURCE_DIR}/Dependencies"
            ${OPENSSL_INCLUDE_DIR}
            ${Boost_INCLUDE_DIRS}
            ${SQLite3_INCLUDE_DIRS}
        )
        target_link_libraries(${exename}
            HOME
            LIBHOMESERVER
            XXHASH
            LIBQUICKMAIL
            dl
            ${CMAKE_THREAD_LIBS_INIT}
            ${OPENSSL_LIBRARIES}
            ${Boost_LIBRARIES}
            ${SQLite3_LIBRARIES}
        )

        add_test(NAME ${testname} COMMAND $<TARGET_FILE:${exename}>)
    endforeach(testfile ${TEST_SOURCES})
endif()
