#include "service.hpp"
#include "home.hpp"
#include "room.hpp"
#include <database/database.hpp>
#include <scripting/script.hpp>
#include <scripting/script_manager.hpp>
#include <scripting/script_source.hpp>

namespace server
{
    namespace main
    {
        Service::Service(identifier_t id, const std::string& name) : Entity(id, name)
        {
        }
        Service::~Service()
        {
        }
        Ref<Service> Service::Create(identifier_t id, const std::string& name)
        {
            Ref<Service> service = boost::make_shared<Service>(id, name);

            if (service != nullptr)
            {
                // Initialize view
                service->view = boost::make_shared<ServiceView>(service);
                if (service->view == nullptr)
                {
                    LOG_ERROR("Failed to initialize service view.");
                    return nullptr;
                }
            }

            return service;
        }

        void Service::JsonGetAttributes(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            (void)output;
            (void)allocator;
        }
        bool Service::JsonSetAttributes(const rapidjson::Value& input)
        {
            (void)input;

            return false;
        }

        // bool Service::SetScriptSourceID(identifier_t scriptSourceID)
        // {
        //     Ref<Database> database = Database::GetInstance();
        //     assert(database != nullptr);

        //     if (database->UpdateServicePropScriptSource(id, scriptSourceID))
        //     {
        //         if (scriptSourceID != 0)
        //         {
        //             // Create script
        //             Ref<scripting::ScriptManager> scriptManager = scripting::ScriptManager::GetInstance();
        //             assert(scriptManager != nullptr);

        //             script = scriptManager->CreateServiceScript(scriptSourceID, view);
        //             if (script == nullptr)
        //             {
        //                 LOG_ERROR("Create service script '{0}'", scriptSourceID);
        //                 return false;
        //             }

        //             // Initialize script
        //             script->Initialize();

        //             return true;
        //         }
        //         else
        //         {
        //             script = nullptr;
        //             return true;
        //         }
        //     }

        //     return false;
        // }
        // identifier_t Service::GetScriptSourceID()
        // {
        //     if (script != nullptr)
        //         return script->GetSourceID();

        //     return 0;
        // }

        // Ref<ServiceView> Service::GetView()
        // {
        //     return view;
        // }

        // void Service::Invoke(const std::string& id, const scripting::sdk::Value& parameter)
        // {
        //     if (script != nullptr)
        //         script->PostInvoke(id, parameter);
        // }

        // void Service::Update()
        // {
        //     if (script != nullptr)
        //         script->PostUpdate(0);
        // }

        // void Service::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        // {
        //     assert(output.IsObject());

        //     output.MemberReserve(4, allocator);

        //     // Build properties
        //     output.AddMember("id", rapidjson::Value(id), allocator);

        //     output.AddMember("name", rapidjson::Value(name.data(), name.size(), allocator), allocator);

        //     output.AddMember("scriptsourceid",
        //                      script != nullptr ? rapidjson::Value(script->GetSourceID())
        //                                        : rapidjson::Value(rapidjson::kNullType),
        //                      allocator);

        //     // Script
        //     rapidjson::Value scriptJson = rapidjson::Value(rapidjson::kObjectType);

        //     if (script != nullptr)
        //         script->JsonGet(scriptJson, allocator);
        //     else
        //         scriptJson.SetNull();

        //     output.AddMember("script", scriptJson, allocator);
        // }
        // void Service::JsonSet(rapidjson::Value& input)
        // {
        //     assert(input.IsObject());

        //     Ref<Home> home = Home::GetInstance();
        //     assert(home != nullptr);

        //     // Decode properties
        //     rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
        //     if (nameIt != input.MemberEnd() && nameIt->value.IsString())
        //         SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

        //     rapidjson::Value::MemberIterator scriptSourceIDIt = input.FindMember("scriptsourceid");
        //     if (scriptSourceIDIt != input.MemberEnd() && scriptSourceIDIt->value.IsUint())
        //         SetScriptSourceID(scriptSourceIDIt->value.GetUint());
        // }

        // void Service::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        // {
        //     assert(output.IsObject());

        //     // Build properties
        //     output.AddMember("id", rapidjson::Value(id), allocator);

        //     rapidjson::Value state = rapidjson::Value(rapidjson::kObjectType);

        //     if (script != nullptr)
        //         script->JsonGetState(state, allocator);

        //     output.AddMember("state", state, allocator);
        // }
        // void Service::JsonSetState(rapidjson::Value& input)
        // {
        //     assert(input.IsObject());

        //     if (script != nullptr)
        //         script->JsonSetState(input);
        // }

        ServiceView::ServiceView(const Ref<Service>& service) : service(service)
        {
            assert(service != nullptr);
        }
        ServiceView::~ServiceView()
        {
        }

        identifier_t ServiceView::GetID() const
        {
            if (Ref<Service> r = service.lock())
                return r->GetID();

            return 0;
        }

        std::string ServiceView::GetName() const
        {
            if (Ref<Service> r = service.lock())
                return r->GetName();

            return "";
        }
        void ServiceView::SetName(const std::string& v)
        {
            if (Ref<Service> r = service.lock())
                r->SetName(v);
        }

        void ServiceView::Invoke(const std::string& method, const scripting::sdk::Value& parameter)
        {
            if (Ref<Service> r = service.lock())
                r->Invoke(method, parameter);
        }

        void ServiceView::Publish()
        {
            if (Ref<Service> r = service.lock())
                r->Publish();
        }

        void ServiceView::PublishState()
        {
            if (Ref<Service> r = service.lock())
                r->PublishState();
        }
    }
}