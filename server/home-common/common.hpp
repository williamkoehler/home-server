#pragma once
#include <iostream>
#include <fstream>
#include <string>

#define SERVER_VERSION "1.0.0b"

typedef uint32_t identifier_t;

#include "macros.hpp"

#include "tools.hpp"

// Config
#include "Configurations.hpp"

// Logging
#include "Log.hpp"

// Boost
#include "boost.hpp"

// Robin Hood (Hash Table, etc...)
#include <robin_hood.h>

// RapidJSON (Json)
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#define RAPIDJSON_BUFFER_SIZE_SMALL 1000
#define RAPIDJSON_BUFFER_SIZE_MEDIUM 50000
#define RAPIDJSON_BUFFER_SIZE_LARGE 100000

// C runtime extension
namespace rapidjson
{
    typedef GenericDocument<UTF8<>, CrtAllocator> CrtDocument;
    typedef GenericValue<UTF8<>, CrtAllocator> CrtValue;
}

// XXHash (Fast hashing)
#include <xxhash.h>

// Memory Management
#include "memory.hpp"

// Signals2
#include <boost/signals2.hpp>

#include "ApiContext.hpp"