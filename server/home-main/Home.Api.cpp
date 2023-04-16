#include "Home.hpp"
#include <home-api/WebSocketSession.hpp>
#include <home-api/User.hpp>

namespace server
{
    namespace main
    {
        void Home::WebSocketProcessGetHomeMessage(const Ref<api::User>& user, const api::ApiRequestMessage& request,
                                               api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)user;
            (void)session;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                home->JsonGet(output, allocator);
            }
        }

        void Home::WebSocketProcessAddEntityMessage(const Ref<api::User>& user, const api::ApiRequestMessage& request,
                                                 api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator typeIt = input.FindMember("type");
            rapidjson::Value::ConstMemberIterator nameIt = input.FindMember("name");
            rapidjson::Value::ConstMemberIterator scriptSourceIdIt = input.FindMember("scriptsourceid");
            if (typeIt == input.MemberEnd() || !typeIt->value.IsString() || // type
                nameIt == input.MemberEnd() || !nameIt->value.IsString() || // name
                scriptSourceIdIt == input.MemberEnd() ||
                (!scriptSourceIdIt->value.IsUint() && !scriptSourceIdIt->value.IsNull())) // scriptsourceid
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Add new entity
                Ref<main::Entity> entity = home->AddEntity(
                    main::ParseEntityType(std::string(typeIt->value.GetString(), typeIt->value.GetStringLength())),
                    std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()),
                    scriptSourceIdIt->value.IsUint() ? scriptSourceIdIt->value.GetUint() : 0, input);
                if (entity == nullptr)
                {
                    //! Error failed to add entity
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }

                entity->JsonGet(output, allocator);
            }
        }
        void Home::WebSocketProcessRemoveEntityMessage(const Ref<api::User>& user, const api::ApiRequestMessage& request,
                                                    api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < api::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Remove device
                if (!home->RemoveEntity(entityIdIt->value.GetUint()))
                {
                    //! Error failed to remove device
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }
            }
        }

        void Home::WebSocketProcessGetEntityMessage(const Ref<api::User>& user, const api::ApiRequestMessage& request,
                                                 api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)user;
            (void)session;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                entity->JsonGet(output, allocator);
            }
        }
        void Home::WebSocketProcessSetEntityMessage(const Ref<api::User>& user, const api::ApiRequestMessage& request,
                                                 api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)user;
            (void)session;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                entity->JsonSet(input);
                entity->JsonGet(output, allocator);
            }
        }

        void Home::WebSocketProcessGetEntityStateMessage(const Ref<api::User>& user, const api::ApiRequestMessage& request,
                                                      api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)user;
            (void)session;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Get state
                rapidjson::Value stateJson = rapidjson::Value(rapidjson::kObjectType);
                entity->JsonGetState(stateJson, allocator);
                output.AddMember("state", stateJson, allocator);
            }
        }
        void Home::WebSocketProcessSetEntityStateMessage(const Ref<api::User>& user, const api::ApiRequestMessage& request,
                                                      api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)user;
            (void)session;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            rapidjson::Value::ConstMemberIterator stateIt = input.FindMember("state");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint() || stateIt == input.MemberEnd() ||
                !stateIt->value.IsObject())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Set state
                entity->JsonSetState(stateIt->value);
            }
        }

        void Home::WebSocketProcessInvokeDeviceMethodMessage(const Ref<api::User>& user,
                                                          const api::ApiRequestMessage& request,
                                                          api::ApiResponseMessage& response, const Ref<api::WebSocketSession>& session)
        {
            (void)user;
            (void)session;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            rapidjson::Value::ConstMemberIterator methodIt = input.FindMember("method");
            rapidjson::Value::ConstMemberIterator parameterIt = input.FindMember("parameter");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint() || methodIt == input.MemberEnd() ||
                !methodIt->value.IsString() || parameterIt == input.MemberEnd())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Invoke method
                entity->Invoke(std::string(methodIt->value.GetString(), methodIt->value.GetStringLength()),
                               scripting::Value::Create(parameterIt->value));
            }
        }

        void Home::WebSocketProcessSubscribeToEntityStateMessage(const Ref<api::User>& user,
                                                              const api::ApiRequestMessage& request,
                                                              api::ApiResponseMessage& response,
                                                              const Ref<api::WebSocketSession>& session)
        {
            (void)user;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Add subscription
                entity->Subscribe(session);

                // Get state
                rapidjson::Value stateJson = rapidjson::Value(rapidjson::kObjectType);
                entity->JsonGetState(stateJson, allocator);
                output.AddMember("state", stateJson, allocator);
            }
        }

        void Home::WebSocketProcessUnsubscribeFromEntityStateMessage(const Ref<api::User>& user,
                                                                  const api::ApiRequestMessage& request,
                                                                  api::ApiResponseMessage& response,
                                                                  const Ref<api::WebSocketSession>& session)
        {
            (void)user;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(api::ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Add subscription
                entity->Unsubscribe(session);
            }
        }
    }
}