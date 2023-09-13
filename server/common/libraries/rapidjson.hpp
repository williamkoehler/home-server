#pragma once
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