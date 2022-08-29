if(${ENABLE_TESTS})
    message(STATUS "Using tests")

    # All test files
    set(TEST_SOURCES

        # Home.Threading
        "${PROJECT_SOURCE_DIR}/server/tests/home-threading/TestWorker.cpp"

        # Home.Database
        "${PROJECT_SOURCE_DIR}/server/tests/home-database/TestDatabase.cpp"
        "${PROJECT_SOURCE_DIR}/server/tests/home-database/TestDatabase-random.cpp"
    )

    # Add one test for each test file
    foreach(testfile ${TEST_SOURCES})
        string(REGEX REPLACE ".cpp\$" "" exename ${testfile})
        string(REGEX REPLACE "^${PROJECT_SOURCE_DIR}/server/tests/" "" exename ${exename})
        string(REPLACE "/" "" exename ${exename})
        string(REGEX REPLACE "([A-Z])" "-\\1" exename ${exename})
        string(TOLOWER ${exename} exename)
        message(STATUS "Found " ${exename})
        string(REPLACE "-" "_" testname ${exename})

        add_executable(${testname} "common.hpp" ${testfile})
        set_target_properties(${testname} PROPERTIES OUTPUT_NAME ${exename})
    
        # Add necessary libraries
        target_include_directories(${testname}
            PUBLIC
            "${PROJECT_SOURCE_DIR}/dependencies"
            "${PROJECT_SOURCE_DIR}/server"
            ${OPENSSL_INCLUDE_DIR}
            ${Boost_INCLUDE_DIRS}
            ${SQLite3_INCLUDE_DIRS}
        )
        target_link_libraries(${testname}
            home_common
            home_threading
            home_database
            dl
            ${CMAKE_THREAD_LIBS_INIT}
            ${OPENSSL_LIBRARIES}
            ${Boost_LIBRARIES}
            ${SQLite3_LIBRARIES}
        )

        add_test(NAME ${testname} COMMAND $<TARGET_FILE:${testname}>)
    endforeach(testfile ${TEST_SOURCES})
endif()
