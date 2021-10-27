#pragma once
#include "../common.hpp"

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