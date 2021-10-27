#pragma once
#include "common.hpp"

namespace server
{
	class ApiContext
	{
	private:
		// Error list containing error or warnings that happened during execution
		rapidjson::Value& json;
		rapidjson::Document::AllocatorType& allocator;

	public:
		ApiContext(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
		~ApiContext();

		void Info(const char* format, ...);
		void Warning(const char* format, ...);
		void Error(const char* format, ...);
	};
}