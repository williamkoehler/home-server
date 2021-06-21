#pragma once
#include "../common.h"

typedef size_t identifier_t;

namespace home
{
	struct Endpoint
	{
		std::string host;
		uint16_t port;
	};

	struct Color
	{
		uint8_t red, green, blue;
	};
}