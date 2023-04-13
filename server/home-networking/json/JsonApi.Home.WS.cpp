#include "JsonApi.hpp"
#include <home-main/Device.hpp>
#include <home-main/Room.hpp>
#include <home-main/Service.hpp>

namespace server
{
    namespace networking
    {
        //! Home
        void JsonApi::ProcessJsonGetHomeMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                  ApiResponseMessage& response, const Ref<ApiSession>& session)
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

        //! Entity
        void JsonApi::ProcessJsonAddEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                    ApiResponseMessage& response, const Ref<ApiSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
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
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }

                entity->JsonGet(output, allocator);
            }
        }
        void JsonApi::ProcessJsonRemoveEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                       ApiResponseMessage& response, const Ref<ApiSession>& session)
        {
            (void)session;

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_AccessLevelToLow);
                return;
            }

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InternalError);
                    return;
                }
            }
        }

        void JsonApi::ProcessJsonGetEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                    ApiResponseMessage& response, const Ref<ApiSession>& session)
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
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                entity->JsonGet(output, allocator);
            }
        }
        void JsonApi::ProcessJsonSetEntityMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                    ApiResponseMessage& response, const Ref<ApiSession>& session)
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
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                entity->JsonSet(input);
                entity->JsonGet(output, allocator);
            }
        }

        void JsonApi::ProcessJsonInvokeDeviceMethodMessageWS(const Ref<users::User>& user,
                                                             const ApiRequestMessage& request,
                                                             ApiResponseMessage& response,
                                                             const Ref<ApiSession>& session)
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
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Invoke method
                entity->Invoke(std::string(methodIt->value.GetString(), methodIt->value.GetStringLength()),
                               scripting::Value::Create(parameterIt->value));
            }
        }

        void JsonApi::ProcessJsonSubscribeToEntityStateMessageWS(const Ref<users::User>& user,
                                                                 const ApiRequestMessage& request,
                                                                 ApiResponseMessage& response,
                                                                 const Ref<ApiSession>& session)
        {
            (void)user;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Add subscription
                entity->Subscribe(session);
            }
        }

        void JsonApi::ProcessJsonUnsubscribeFromEntityStateMessageWS(const Ref<users::User>& user,
                                                                     const ApiRequestMessage& request,
                                                                     ApiResponseMessage& response,
                                                                     const Ref<ApiSession>& session)
        {
            (void)user;

            const rapidjson::Document& input = request.GetJsonDocument();
            rapidjson::Document& output = response.GetJsonDocument();
            rapidjson::Document::AllocatorType& allocator = response.GetJsonAllocator();

            // Process request
            rapidjson::Value::ConstMemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                // Add subscription
                entity->Unsubscribe(session);
            }
        }

        void JsonApi::ProcessJsonGetEntityStateMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                         ApiResponseMessage& response, const Ref<ApiSession>& session)
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
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
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
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }

                Ref<scripting::Script> script = entity->GetScript();
                if (script != nullptr)
                {
                    // Get state
                    script->JsonGetProperties(output, allocator);
                }
            }
        }
        void JsonApi::ProcessJsonSetEntityStateMessageWS(const Ref<users::User>& user, const ApiRequestMessage& request,
                                                         ApiResponseMessage& response, const Ref<ApiSession>& session)
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
                response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidArguments);
                return;
            }

            // Build response
            {
                Ref<main::Home> home = main::Home::GetInstance();
                assert(home != nullptr);

                // Get entity
                Ref<main::Device> entity = home->GetDevice(entityIdIt->value.GetUint());
                if (entity == nullptr)
                {
                    response.SetErrorCode(ApiErrorCodes::kApiErrorCode_InvalidIdentifier);
                    return;
                }
            }
        }
    }
}