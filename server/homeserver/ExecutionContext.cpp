#include "ExecutionContext.hpp"

namespace server
{
	ExecutionContext::ExecutionContext(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
		: json(json), allocator(allocator)
	{
		assert(json.IsArray());
	}
	ExecutionContext::~ExecutionContext()
	{
	}

	void ExecutionContext::Info(const char* format, ...)
	{
		char buf[58] = "INFO : ";

		va_list lst;
		va_start(lst, format);
		const size_t len = snprintf(buf + 7, 50, format, lst);
		va_end(lst);

		json.PushBack(rapidjson::Value(buf, len + 7, allocator), allocator);
		LOG_INFO((buf + 7));
	}
	void ExecutionContext::Warning(const char* format, ...)
	{
		char buf[61] = "WARNING : ";

		va_list lst;
		va_start(lst, format);
		const size_t len = snprintf(buf + 10, 50, format, lst);
		va_end(lst);

		json.PushBack(rapidjson::Value(buf, len + 10, allocator), allocator);
		LOG_WARNING((buf + 10));
	}
	void ExecutionContext::Error(const char* format, ...)
	{
		char buf[59] = "ERROR : ";

		va_list lst;
		va_start(lst, format);
		const size_t len = snprintf(buf + 8, 50, format, lst);
		va_end(lst);

		json.PushBack(rapidjson::Value(buf, len + 8, allocator), allocator);
		LOG_ERROR((buf + 8));
	}
}