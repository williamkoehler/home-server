#pragma once
#include <iostream>
#include <string>
#include <ostream>

typedef int64_t identifier_t;

#include "Log.hpp"

//Dependencies
#include "boost.hpp"
#include <robin-hood/robin_hood.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/error/en.h>
#define RAPIDJSON_BUFFER_SIZE_SMALL 1000
#define RAPIDJSON_BUFFER_SIZE_MEDIUM 50000
#define RAPIDJSON_BUFFER_SIZE_LARGE 100000

// C runtime extension
namespace rapidjson
{
	typedef GenericDocument<UTF8<>, CrtAllocator> CrtDocument;
	typedef GenericValue<UTF8<>, CrtAllocator> CrtValue;
}

//HASH
#include <xxhash/xxhash.h>

#include "memory.hpp"