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
            static void ProcessJsonGetUsersMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                     rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonAddUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonRemoveUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                       rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonGetUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonSetUserMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context);

            // Home
            static void ProcessJsonGetHomeMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context);

            // Room
            static void ProcessJsonAddRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonRemoveRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                       rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonGetRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonSetRoomMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context);

            // Device
            static void ProcessJsonAddDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                      rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonRemoveDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                         rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonGetDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                      rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonSetDeviceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                      rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonInvokeDeviceEventMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                              rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonGetDeviceStateMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                           rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonSetDeviceStateMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                           rapidjson::Document& output, ApiContext& context);

            // Scripting
            static void ProcessJsonGetScriptSourcesMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                             rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonAddScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                            rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonRemoveScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                               rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonGetScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                            rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonSetScriptSourceMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                            rapidjson::Document& output, ApiContext& context);

            static void ProcessJsonGetScriptSourceContentMessageWS(const Ref<users::User>& user,
                                                                   rapidjson::Document& input,
                                                                   rapidjson::Document& output, ApiContext& context);
            static void ProcessJsonSetScriptSourceContentMessageWS(const Ref<users::User>& user,
                                                                   rapidjson::Document& input,
                                                                   rapidjson::Document& output, ApiContext& context);
        };
    }
}