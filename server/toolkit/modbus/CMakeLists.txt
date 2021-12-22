cmake_minimum_required(VERSION 3.8)

set(modbus_src
"modbus.hpp"
"modbus.cpp"
)

add_library(modbus SHARED ${modbus_src})
set_target_properties(modbus PROPERTIES OUTPUT_NAME "modbus")

target_compile_definitions(modbus
    PUBLIC
    MODBUS_COMPILE
)