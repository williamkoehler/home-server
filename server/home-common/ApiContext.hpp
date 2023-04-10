#pragma once
#include "common.hpp"

namespace server
{
    enum ApiError
    {
        kError_NoError = 0,
        kError_InternalError,
        kError_InvalidArguments,
        kError_AccessLevelToLow,
        kError_InvalidIdentifier,

        //! User
        kError_UserComponent = 10000,
        kError_UserInvalidName,
        kError_UserInvalidPassword,

        //! Scripting
        kError_ScriptingComponent = 20000,

        //! Home
        kError_HomeComponent = 30000,

        //! Room
        kError_RoomComponent = 40000,
        kError_RoomInvalidName,
        kError_RoomInvalidType,

        //! DeviceController
        kError_DeviceControllerComponent = 50000,
        kError_DeviceControllerInvalidName,
        kError_DeviceControllerInvalidPluginID,

        //! Device
        kError_DeviceComponent = 60000,
        kError_DeviceInvalidName,
        kError_DeviceInvalidPluginID,
    };

    class ApiContext
    {
      public:
        ApiContext()
        {
        }
        virtual ~ApiContext()
        {
        }

        virtual void Info(const char* format, ...) = 0;
        virtual void Warning(const char* format, ...) = 0;
        virtual void Error(const char* format, ...) = 0;

        virtual ApiError GetError() = 0;
        virtual void Error(ApiError message) = 0;
    };

    class EmptyApiContext : public ApiContext
    {
      public:
        EmptyApiContext()
        {
        }
        virtual ~EmptyApiContext()
        {
        }

        virtual void Info(const char* format, ...) override
        {
            (void)format;
        }
        virtual void Warning(const char* format, ...) override
        {
            (void)format;
        }
        virtual void Error(const char* format, ...) override
        {
            (void)format;
        }

        virtual ApiError GetError() override
        {
            return ApiError::kError_NoError;
        }
        virtual void Error(ApiError message) override
        {
            (void)message;
        }
    };
}