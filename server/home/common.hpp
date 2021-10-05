#pragma once
#include <iostream>
#include <string>
#include <ostream>

#include "Log.hpp"

//Dependencies
#include "boost.hpp"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/error/en.h>
#define RAPIDJSON_BUFFER_SIZE_SMALL 1000
#define RAPIDJSON_BUFFER_SIZE_MEDIUM 50000
#define RAPIDJSON_BUFFER_SIZE_LARGE 100000

//HASH
#include <xxHash/xxhash.h>

#include "memory.hpp"