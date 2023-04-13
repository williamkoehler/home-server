#pragma once
#include "../common.hpp"
#include <home-main/Home.hpp>
#include <home-scripting/ScriptSource.hpp>
#include <home-users/User.hpp>

namespace server
{
    namespace networking
    {
        class JsonApi
        {
          public: // HTTP
            // Build
            static void BuildJsonErrorMessageHTTP(const char* error, rapidjson::Document& output);

            // Process
            static bool ProcessResApiCallHTTP(boost::beast::http::verb verb, std::string_view target,
                                              const Ref<users::User>& user, std::string_view input,
                                              rapidjson::StringBuffer& output, std::string& contentType);

          public: // WS
            // User
            static void ProcessJsonGetUsersMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                     ApiResponseMessage& response, const Ref<ApiSession>& session);

            static void ProcessJsonAddUserMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                    ApiResponseMessage& response, const Ref<ApiSession>& session);
            static void ProcessJsonRemoveUserMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                       ApiResponseMessage& response, const Ref<ApiSession>& session);

            static void ProcessJsonGetUserMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                    ApiResponseMessage& response, const Ref<ApiSession>& session);
            static void ProcessJsonSetUserMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                    ApiResponseMessage& response, const Ref<ApiSession>& session);

            // Home
            static void ProcessJsonGetHomeMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                    ApiResponseMessage& response, const Ref<ApiSession>& session);

            // Entity
            static void ProcessJsonAddEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                      ApiResponseMessage& response, const Ref<ApiSession>& session);
            static void ProcessJsonRemoveEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                         ApiResponseMessage& response, const Ref<ApiSession>& session);

            static void ProcessJsonGetEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                      ApiResponseMessage& response, const Ref<ApiSession>& session);
            static void ProcessJsonSetEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                      ApiResponseMessage& response, const Ref<ApiSession>& session);

            static void ProcessJsonInvokeDeviceMethodMessageWS(const Ref<users::User>& user,
                                                               const ApiRequestMessage& request,
                                                               ApiResponseMessage& response,
                                                               const Ref<ApiSession>& session);

            static void ProcessJsonSubscribeToEntityStateMessageWS(const Ref<users::User>& user,
                                                                   const ApiRequestMessage& request,
                                                                   ApiResponseMessage& response,
                                                                   const Ref<ApiSession>& session);

            static void ProcessJsonUnsubscribeFromEntityStateMessageWS(const Ref<users::User>& user,
                                                                     const ApiRequestMessage& request,
                                                                     ApiResponseMessage& response,
                                                                     const Ref<ApiSession>& session);

            static void ProcessJsonGetEntityStateMessageWS(const Ref<users::User>& user,
                                                           const ApiRequestMessage& request,
                                                           ApiResponseMessage& response,
                                                           const Ref<ApiSession>& session);
            static void ProcessJsonSetEntityStateMessageWS(const Ref<users::User>& user,
                                                           const ApiRequestMessage& request,
                                                           ApiResponseMessage& response,
                                                           const Ref<ApiSession>& session);

            // Scripting
            static void ProcessJsonGetScriptSourcesMessageWS(const Ref<users::User>& user,
                                                             const ApiRequestMessage& request,
                                                             ApiResponseMessage& response,
                                                             const Ref<ApiSession>& session);

            static void ProcessJsonAddScriptSourceMessageWS(const Ref<users::User>& user,
                                                            const ApiRequestMessage& request,
                                                            ApiResponseMessage& response,
                                                            const Ref<ApiSession>& session);
            static void ProcessJsonRemoveScriptSourceMessageWS(const Ref<users::User>& user,
                                                               const ApiRequestMessage& request,
                                                               ApiResponseMessage& response,
                                                               const Ref<ApiSession>& session);

            static void ProcessJsonGetScriptSourceMessageWS(const Ref<users::User>& user,
                                                            const ApiRequestMessage& request,
                                                            ApiResponseMessage& response,
                                                            const Ref<ApiSession>& session);
            static void ProcessJsonSetScriptSourceMessageWS(const Ref<users::User>& user,
                                                            const ApiRequestMessage& request,
                                                            ApiResponseMessage& response,
                                                            const Ref<ApiSession>& session);

            static void ProcessJsonGetScriptSourceContentMessageWS(const Ref<users::User>& user,
                                                                   const ApiRequestMessage& request,
                                                                   ApiResponseMessage& response,
                                                                   const Ref<ApiSession>& session);
            static void ProcessJsonSetScriptSourceContentMessageWS(const Ref<users::User>& user,
                                                                   const ApiRequestMessage& request,
                                                                   ApiResponseMessage& response,
                                                                   const Ref<ApiSession>& session);
        };
    }
}