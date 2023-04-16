#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#define SERVER_VERSION "1.0.0b"

typedef uint32_t identifier_t;

#define SHA256_SIZE 32
#define SALT_SIZE 16

#include "Helper.hpp"

#include "config.hpp"

// Logging
#include "Log.hpp"

// Libraries
#include "libraries/boost.hpp"
#include "libraries/memory.hpp"
#include "libraries/rapidjson.hpp"
#include <robin_hood.h>
#include <xxhash.h>