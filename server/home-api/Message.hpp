#pragma once
#include "common.hpp"

namespace server
{
    namespace api
    {
        class User;
        class WebSocketSession;

        enum ApiErrorCodes : uint32_t
        {
            kApiErrorCode_NoError = 0,
            kApiErrorCode_InternalError,
            kApiErrorCode_InvalidArguments,
            kApiErrorCode_AccessLevelToLow,
            kApiErrorCode_InvalidIdentifier,

            //! User
            kApiErrorCode_UserError = 10000,
            kApiErrorCode_UserInvalidName,
            kApiErrorCode_UserInvalidPassword,

            //! Scripting
            kApiErrorCode_ScriptError = 20000,

            //! Home
            kApiErrorCode_HomeError = 30000,

            //! Entity
            kApiErrorCode_EntityError = 40000,
            kApiErrorCode_InvalidEntityType,
            kApiErrorCode_InvalidEntityName,
            kApiErrorCode_InvalidRoomType,
            kApiErrorCode_InvalidDeviceRoomId,
        };

        class ApiMessage
        {
          protected:
            rapidjson::Document document;

            ApiMessage() : document(rapidjson::kObjectType)
            {
            }
            ApiMessage(rapidjson::Document document) : document(std::move(document))
            {
            }

          public:
            ApiMessage(const ApiMessage& apiMessage) = delete;

            /// @brief Get json document
            ///
            /// @return rapidjson::Document& Json document
            inline rapidjson::Document& GetJsonDocument()
            {
                return document;
            }

            /// @brief Get json document
            ///
            /// @return rapidjson::Document& Json document
            inline const rapidjson::Document& GetJsonDocument() const
            {
                return document;
            }

            /// @brief Get json document
            ///
            /// @return rapidjson::Document& Json document
            inline rapidjson::Document::AllocatorType& GetJsonAllocator()
            {
                return document.GetAllocator();
            }
        };

        class ApiRequestMessage final : public ApiMessage
        {
          private:
            std::string type;

          public:
            ApiRequestMessage()
            {
            }
            ApiRequestMessage(const std::string& type) : type(type)
            {
            }
            ApiRequestMessage(const std::string& type, rapidjson::Document document)
                : ApiMessage(std::move(document)), type(type)
            {
            }

            /// @brief Get message type
            ///
            /// @return const std::string& Message type
            inline std::string& GetType()
            {
                return type;
            }

            /// @brief Get message type
            ///
            /// @return const std::string& Message type
            inline const std::string& GetType() const
            {
                return type;
            }

            /// @brief Set message type
            ///
            /// @param v Message type
            inline void SetType(const std::string& v)
            {
                type = v;
            }
        };

        class ApiResponseMessage final : public ApiMessage
        {
          private:
            ApiErrorCodes errorCode;

          public:
            inline ApiResponseMessage() : errorCode(kApiErrorCode_NoError)
            {
            }
            inline ApiResponseMessage(ApiErrorCodes errorCode) : errorCode(errorCode)
            {
            }
            inline ApiResponseMessage(ApiErrorCodes errorCode, rapidjson::Document document)
                : ApiMessage(std::move(document)), errorCode(errorCode)
            {
            }

            /// @brief Get message error code
            ///
            /// @return const std::string& Message type
            inline ApiErrorCodes& GetErrorCode()
            {
                return errorCode;
            }

            /// @brief Get message error code
            ///
            /// @return const std::string& Message type
            inline const ApiErrorCodes& GetErrorCode() const
            {
                return errorCode;
            }

            /// @brief Set message error code
            ///
            /// @param v Message type
            inline void SetErrorCode(ApiErrorCodes v)
            {
                errorCode = v;
            }
        };

        class ApiBroadcastMessage final : public ApiMessage
        {
          private:
            std::string type;

          public:
            inline ApiBroadcastMessage()
            {
            }
            inline ApiBroadcastMessage(const std::string& type) : type(type)
            {
            }
            inline ApiBroadcastMessage(const std::string& type, rapidjson::Document document)
                : ApiMessage(std::move(document)), type(type)
            {
            }

            /// @brief Get message type
            ///
            /// @return const std::string& Message type
            inline std::string& GetType()
            {
                return type;
            }

            /// @brief Get message type
            ///
            /// @return const std::string& Message type
            inline const std::string& GetType() const
            {
                return type;
            }

            /// @brief Set message type
            ///
            /// @param v Message type
            inline void SetType(const std::string& v)
            {
                type = v;
            }
        };
    }
}