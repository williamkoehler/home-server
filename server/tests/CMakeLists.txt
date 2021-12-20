if(${ENABLE_TESTS})
    message(STATUS "Using tests")

    # All test files
    set(TEST_SOURCES

        # Database
        "${PROJECT_SOURCE_DIR}/server/tests/homeserver/database/TestTables.cpp"
        "${PROJECT_SOURCE_DIR}/server/tests/homeserver/database/TestRooms.cpp"
        "${PROJECT_SOURCE_DIR}/server/tests/homeserver/database/TestControllers.cpp"
        "${PROJECT_SOURCE_DIR}/server/tests/homeserver/database/TestDevices.cpp"
        "${PROJECT_SOURCE_DIR}/server/tests/homeserver/database/TestUsers.cpp"
    )

    # Add one test for each test file
    foreach(testfile ${TEST_SOURCES})
        string(REPLACE ".cpp" "" testname ${testfile})
        string(REPLACE "${PROJECT_SOURCE_DIR}/server/tests/" "" testname ${testname})
        message(STATUS "Found " ${testname})
        string(REPLACE "/" "-" exename ${testname})

        add_executable(${exename} ${testfile} "common.hpp")
    
        # Add necessary libraries
        target_include_directories(${exename}
            PUBLIC
            "${PROJECT_SOURCE_DIR}/server/home"
            "${PROJECT_SOURCE_DIR}/server/homeserver"
            "${PROJECT_SOURCE_DIR}/server/plugins"
            "${PROJECT_SOURCE_DIR}/dependencies"
            ${OPENSSL_INCLUDE_DIR}
            ${Boost_INCLUDE_DIRS}
            ${SQLite3_INCLUDE_DIRS}
        )
        target_link_libraries(${exename}
            HOME
            HOMESERVER
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
