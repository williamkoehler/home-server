#pragma once
#include "ApiMessage.hpp"
#include "boost.hpp"
#include "memory.hpp"

namespace server
{
    // Server -> Client

    // Client -> Server

    class ApiSession
    {
      protected:
        ApiSession()
        {
        }
        virtual ~ApiSession()
        {
        }

      public:
        virtual void Send(const ApiMessage& message) = 0;
        virtual void Send(const Ref<rapidjson::StringBuffer>& message) = 0;
    };
}