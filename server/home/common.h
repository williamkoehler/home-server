#pragma once
#include <iostream>
#include <string>
#include <ostream>

#include "Log.h"

//Dependencies
#include "boost.h"

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

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#endif

#include "memory.h"

#include "export.h"
