#pragma once
#include "common.hpp"

namespace server
{
	class ExecutionContext
	{
	private:
		// Error list containing error or warnings that happened during execution
		rapidjson::Value& json;
		rapidjson::Document::AllocatorType& allocator;

	public:
		ExecutionContext(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
		~ExecutionContext();

		void Info(const char* format, ...);
		void Warning(const char* format, ...);
		void Error(const char* format, ...);
	};
}