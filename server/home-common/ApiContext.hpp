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
      private:
        // Error list containing error or warnings that happened during execution
        rapidjson::Value& log;
        rapidjson::Document::AllocatorType& allocator;

        ApiError error;

      public:
        ApiContext(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
        virtual ~ApiContext();

        inline ApiError GetError()
        {
            return error;
        }

        void Info(const char* format, ...);
        void Warning(const char* format, ...);
        void Error(const char* format, ...);

        void Error(ApiError message);
    };
}