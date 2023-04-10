#pragma once
#include <home-common/ApiContext.hpp>

namespace server
{
    namespace networking
    {
        class NetworkApiContext : public ApiContext
        {
          private:
            // Error list containing error or warnings that happened during execution
            rapidjson::Value& log;
            rapidjson::Document::AllocatorType& allocator;

            ApiError error;

          public:
            NetworkApiContext(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
            virtual ~NetworkApiContext();

            virtual ApiError GetError() override
            {
                return error;
            }

            virtual void Info(const char* format, ...) override;
            virtual void Warning(const char* format, ...) override;
            virtual void Error(const char* format, ...) override;

            virtual void Error(ApiError message) override;
        };
    }
}