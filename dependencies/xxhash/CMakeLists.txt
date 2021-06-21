cmake_minimum_required(VERSION 3.8)

add_library(XXHASH STATIC "xxhash.c" "xxhash.h")
set_target_properties(XXHASH PROPERTIES OUTPUT_NAME "xxhash")