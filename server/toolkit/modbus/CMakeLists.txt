cmake_minimum_required(VERSION 3.8)

set(modbus_src
"modbus.h"
"modbus.cpp"
)

add_library(MODBUS SHARED ${modbus_src})

target_compile_definitions(MODBUS
    PUBLIC
    MODBUS_COMPILE
)

if (WIN32)
    target_link_libraries(MODBUS
        "ws2_32.lib"
    )
endif (WIN32)