cmake_minimum_required(VERSION 3.8)

set(modbus_src
"modbus.hpp"
"modbus.cpp"
)

add_library(MODBUS SHARED ${modbus_src})
set_target_properties(MODBUS PROPERTIES OUTPUT_NAME "modbus")

target_compile_definitions(MODBUS
    PUBLIC
    MODBUS_COMPILE
)