#include "ApiContext.hpp"

namespace server
{
	ApiContext::ApiContext(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
		: json(json), allocator(allocator)
	{
		assert(json.IsArray());
	}
	ApiContext::~ApiContext()
	{
	}

	#define BUFFER_SIZE 64

	void ApiContext::Info(const char* format, ...)
	{
		char buf[BUFFER_SIZE] = "INFO    : ";

		va_list lst;
		va_start(lst, format);
		const size_t len = snprintf(buf + 10, BUFFER_SIZE - 10, format, lst);
		va_end(lst);

		json.PushBack(rapidjson::Value(buf, len + 10, allocator), allocator);
		LOG_INFO((buf + 7));
	}
	void ApiContext::Warning(const char* format, ...)
	{
		char buf[BUFFER_SIZE] = "WARNING : ";

		va_list lst;
		va_start(lst, format);
		const size_t len = snprintf(buf + 10, BUFFER_SIZE - 10, format, lst);
		va_end(lst);

		json.PushBack(rapidjson::Value(buf, len + 10, allocator), allocator);
		LOG_WARNING((buf + 10));
	}
	void ApiContext::Error(const char* format, ...)
	{
		char buf[BUFFER_SIZE] = "ERROR   : ";

		va_list lst;
		va_start(lst, format);
		const size_t len = snprintf(buf + 10, BUFFER_SIZE - 10, format, lst);
		va_end(lst);

		json.PushBack(rapidjson::Value(buf, len + 10, allocator), allocator);
		LOG_ERROR((buf + 8));
	}
}