#include "ApiContext.hpp"

namespace server
{
    namespace networking
    {
        NetworkApiContext::NetworkApiContext(rapidjson::Value& log, rapidjson::Document::AllocatorType& allocator)
            : log(log), allocator(allocator), error(ApiError::kError_NoError)
        {
            assert(log.IsArray());
        }
        NetworkApiContext::~NetworkApiContext()
        {
        }

#define BUFFER_SIZE 64

        void NetworkApiContext::Info(const char* format, ...)
        {
            char buf[BUFFER_SIZE] = "info   : ";

            va_list lst;
            va_start(lst, format);
            const size_t len = vsnprintf(buf + 9, BUFFER_SIZE - 10, format, lst);
            va_end(lst);

            log.PushBack(rapidjson::Value(buf, len + 9, allocator), allocator);
            LOG_INFO((buf + 9));
        }
        void NetworkApiContext::Warning(const char* format, ...)
        {
            char buf[BUFFER_SIZE] = "warn   : ";

            va_list lst;
            va_start(lst, format);
            const size_t len = vsnprintf(buf + 9, BUFFER_SIZE - 10, format, lst);
            va_end(lst);

            log.PushBack(rapidjson::Value(buf, len + 9, allocator), allocator);
            LOG_WARNING((buf + 9));
        }
        void NetworkApiContext::Error(const char* format, ...)
        {
            char buf[BUFFER_SIZE] = "error  : ";

            va_list lst;
            va_start(lst, format);
            const size_t len = vsnprintf(buf + 9, BUFFER_SIZE - 10, format, lst);
            va_end(lst);

            log.PushBack(rapidjson::Value(buf, len + 9, allocator), allocator);
            LOG_ERROR((buf + 9));
        }

        void NetworkApiContext::Error(ApiError code)
        {
            if (!error)
                error = code;

            Error("(error code %zu)", (uint64_t)code);
        }
    }
}