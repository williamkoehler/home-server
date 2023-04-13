#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

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
#include "rapidjson.hpp"

// XXHash (Fast hashing)
#include <xxhash.h>

// Memory Management
#include "memory.hpp"

// Signals2
#include <boost/signals2.hpp>

#include "ApiMessage.hpp"
#include "ApiSession.hpp"