#include "ApiContext.hpp"

namespace server
{
    ApiContext::ApiContext(rapidjson::Value& log, rapidjson::Document::AllocatorType& allocator)
        : log(log), error(ApiError::kError_NoError), allocator(allocator)
    {
        assert(log.IsArray());
    }
    ApiContext::~ApiContext()
    {
    }

#define BUFFER_SIZE 64

    void ApiContext::Info(const char* format, ...)
    {
        char buf[BUFFER_SIZE] = "info   : ";

        va_list lst;
        va_start(lst, format);
        const size_t len = vsnprintf(buf + 9, BUFFER_SIZE - 10, format, lst);
        va_end(lst);

        log.PushBack(rapidjson::Value(buf, len + 9, allocator), allocator);
        LOG_INFO((buf + 9));
    }
    void ApiContext::Warning(const char* format, ...)
    {
        char buf[BUFFER_SIZE] = "warn   : ";

        va_list lst;
        va_start(lst, format);
        const size_t len = vsnprintf(buf + 9, BUFFER_SIZE - 10, format, lst);
        va_end(lst);

        log.PushBack(rapidjson::Value(buf, len + 9, allocator), allocator);
        LOG_WARNING((buf + 9));
    }
    void ApiContext::Error(const char* format, ...)
    {
        char buf[BUFFER_SIZE] = "error  : ";

        va_list lst;
        va_start(lst, format);
        const size_t len = vsnprintf(buf + 9, BUFFER_SIZE - 10, format, lst);
        va_end(lst);

        log.PushBack(rapidjson::Value(buf, len + 9, allocator), allocator);
        LOG_ERROR((buf + 9));
    }

    void ApiContext::Error(ApiError code)
    {
        if (!error)
            error = code;

        Error("(error code %zu)", (uint64_t)code);
    }
}