cmake_minimum_required(VERSION 3.8)

add_library(xxhash STATIC "xxhash.c" "xxhash.h")
set_target_properties(xxhash PROPERTIES OUTPUT_NAME "xxhash")