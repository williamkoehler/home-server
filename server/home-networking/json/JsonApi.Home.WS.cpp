#include "JsonApi.hpp"
#include <home-main/Device.hpp>
#include <home-main/Room.hpp>
#include <home-main/Service.hpp>

namespace server
{
    namespace networking
    {
        //! Home
        void JsonApi::ProcessJsonGetHomeMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                  rapidjson::Document& output, ApiContext& context)
        {
            (void)context;

            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            home->ApiGet(output, allocator, context);
        }

        //! Entity
        void JsonApi::ProcessJsonAddEntityMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            rapidjson::Value::MemberIterator scriptSourceIdIt = input.FindMember("scriptsourceid");
            if (typeIt == input.MemberEnd() || !typeIt->value.IsString() || // type
                nameIt == input.MemberEnd() || !nameIt->value.IsString() || // name
                scriptSourceIdIt == input.MemberEnd() ||
                (!scriptSourceIdIt->value.IsUint() && !scriptSourceIdIt->value.IsNull())) // scriptsourceid
            {
                context.Error("Missing type, name, and/or scriptsourceid");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

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
                context.Error(ApiError::kError_InternalError);
                return;
            }

            entity->JsonGet(output, allocator);
        }
        void JsonApi::ProcessJsonRemoveEntityMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                       rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            if (user->GetAccessLevel() < users::UserAccessLevel::kMaintainerUserAccessLevel)
            {
                context.Error(ApiError::kError_AccessLevelToLow);
                return;
            }

            // Process request
            rapidjson::Value::MemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Remove device
            if (!home->RemoveEntity(entityIdIt->value.GetUint()))
            {
                //! Error failed to remove device
                context.Error(ApiError::kError_InternalError);
                return;
            }
        }

        void JsonApi::ProcessJsonGetEntityMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get entity
            Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
            if (entity == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            entity->ApiGet(output, allocator, context);
        }
        void JsonApi::ProcessJsonSetEntityMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                    rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get entity
            Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
            if (entity == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            entity->ApiSet(input, context);
            entity->ApiGet(output, allocator, context);
        }

        void JsonApi::ProcessJsonInvokeDeviceMethodMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                             rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator entityIdIt = input.FindMember("id");
            rapidjson::Value::MemberIterator methodIt = input.FindMember("method");
            rapidjson::Value::MemberIterator parameterIt = input.FindMember("parameter");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint() || methodIt == input.MemberEnd() ||
                !methodIt->value.IsString() || parameterIt == input.MemberEnd())
            {
                context.Error("Missing id and/or method");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get entity
            Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
            if (entity == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            // Invoke method
            entity->ApiInvoke(std::string(methodIt->value.GetString(), methodIt->value.GetStringLength()),
                              scripting::Value::Create(parameterIt->value));
        }

        void JsonApi::ProcessJsonGetEntityStateMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                         rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator entityIdIt = input.FindMember("id");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint())
            {
                context.Error("Missing id");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get entity
            Ref<main::Entity> entity = home->GetEntity(entityIdIt->value.GetUint());
            if (entity == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            rapidjson::Value state = rapidjson::Value(rapidjson::kObjectType);
            entity->ApiGetState(state, allocator, context);
            output.AddMember("state", state, allocator);
        }
        void JsonApi::ProcessJsonSetEntityStateMessageWS(const Ref<users::User>& user, rapidjson::Document& input,
                                                         rapidjson::Document& output, ApiContext& context)
        {
            assert(user != nullptr);
            assert(input.IsObject() && output.IsObject());

            // Process request
            rapidjson::Value::MemberIterator entityIdIt = input.FindMember("id");
            rapidjson::Value::MemberIterator stateIt = input.FindMember("state");
            if (entityIdIt == input.MemberEnd() || !entityIdIt->value.IsUint() || stateIt == input.MemberEnd() ||
                !stateIt->value.IsObject())
            {
                context.Error("Missing id and/or state");
                context.Error(ApiError::kError_InvalidArguments);
                return;
            }

            // Build response
            rapidjson::Document::AllocatorType& allocator = output.GetAllocator();

            Ref<main::Home> home = main::Home::GetInstance();
            assert(home != nullptr);

            // Get entity
            Ref<main::Device> entity = home->GetDevice(entityIdIt->value.GetUint());
            if (entity == nullptr)
            {
                context.Error(ApiError::kError_InvalidIdentifier);
                return;
            }

            entity->ApiSetState(stateIt->value, context);

            rapidjson::Value state = rapidjson::Value(rapidjson::kObjectType);
            entity->ApiGetState(state, allocator, context);
            output.AddMember("state", state, allocator);
        }
    }
}