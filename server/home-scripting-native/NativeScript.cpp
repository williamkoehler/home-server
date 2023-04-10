#include "NativeScript.hpp"
#include "NativeScriptSource.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScript::NativeScript(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource,
                                       const Ref<NativeScriptImpl>& scriptImpl)
                : Script(view, scriptSource), scriptImpl(scriptImpl)
            {
            }
            NativeScript::~NativeScript()
            {
            }
            Ref<Script> NativeScript::Create(const Ref<View>& view, const Ref<NativeScriptSource>& scriptSource)
            {
                Ref<NativeScriptImpl> scriptImpl = scriptSource->GetCreateCallback()();

                if (scriptImpl != nullptr)
                {
                    Ref<NativeScript> script =
                        boost::make_shared<NativeScript>(view, scriptSource, std::move(scriptImpl));

                    // Initialize script implementation
                    script->scriptImpl->script = script;

                    return script->shared_from_this();
                }
                else
                    return nullptr;
            }

            bool NativeScript::Initialize()
            {
                return scriptImpl->Initialize();
            }

            bool NativeScript::AddAttribute(const std::string& name, const char* json)
            {
                // Check existance
                if (!attributeMap.contains(name))
                {
                    // Parse attribute
                    rapidjson::Document document;

                    document.Parse(json);

                    // Add attribute to list
                    if (!document.HasParseError())
                    {
                        attributeMap[name] = std::move(document);
                        return true;
                    }
                }

                return false;
            }
            bool NativeScript::RemoveAttribute(const std::string& name)
            {
                return attributeMap.erase(name);
            }
            void NativeScript::ClearAttributes()
            {
                attributeMap.clear();
            }

            bool NativeScript::AddProperty(const std::string& name, UniqueRef<Property> property)
            {
                if (!propertyMap.contains(name))
                {
                    propertyMap[name] = std::move(property);
                    return true;
                }
                else
                    return false;
            }

            Value NativeScript::GetProperty(const std::string& name)
            {
                robin_hood::unordered_node_map<std::string, UniqueRef<Property>>::const_iterator it =
                    propertyMap.find(name);
                if (it != propertyMap.end())
                    return it->second->Get(scriptImpl.get());
                else
                    return Value();
            }

            void NativeScript::SetProperty(const std::string& name, const Value& value)
            {
                robin_hood::unordered_node_map<std::string, UniqueRef<Property>>::const_iterator it =
                    propertyMap.find(name);
                if (it != propertyMap.end())
                    it->second->Set(scriptImpl.get(), value);
            }

            bool NativeScript::RemoveProperty(const std::string& name)
            {
                return propertyMap.erase(name);
            }
            void NativeScript::ClearProperties()
            {
                propertyMap.clear();
            }

            bool NativeScript::AddMethod(const std::string& name, UniqueRef<Method> method)
            {
                if (!methodMap.contains(name))
                {
                    methodMap[name] = std::move(method);
                    return true;
                }
                else
                    return false;
            }

            bool NativeScript::Invoke(const std::string& name, const Value& parameter)
            {
                robin_hood::unordered_node_map<std::string, UniqueRef<Method>>::const_iterator it =
                    methodMap.find(name);
                if (it != methodMap.end())
                    return it->second->Invoke(scriptImpl.get(), parameter);
                else
                    return false;
            }

            bool NativeScript::RemoveMethod(const std::string& name)
            {
                return methodMap.erase(name);
            }
            void NativeScript::ClearMethods()
            {
                methodMap.clear();
            }

            Event NativeScript::AddEvent(const std::string& name)
            {
                return eventMap.emplace(name, Event()).first->second;
            }
            bool NativeScript::RemoveEvent(const std::string& name)
            {
                return eventMap.erase(name);
            }
            void NativeScript::ClearEvents()
            {
                eventMap.clear();
            }

            void NativeScript::JsonGetProperties(rapidjson::Value& output,
                                                 rapidjson::Document::AllocatorType& allocator, uint8_t propertyFlags)
            {
                assert(output.IsObject());

                output.MemberReserve(propertyMap.size(), allocator);
                for (auto& [name, property] : propertyMap)
                {
                    // Add property
                    if (property->GetFlags() & propertyFlags)
                    {
                        output.AddMember(rapidjson::Value(name.data(), name.size(), allocator),
                                         property->Get(scriptImpl.get()).JsonGet(allocator), allocator);
                    }
                }
            }
            uint8_t NativeScript::JsonSetProperties(const rapidjson::Value& input, uint8_t propertyFlags)
            {
                assert(input.IsObject());

                uint8_t updateFlags = 0; // We use an uint8_t to allow to check what properties where updated

                for (rapidjson::Value::ConstMemberIterator propertyIt = input.MemberBegin(); propertyIt != input.MemberEnd();
                     propertyIt++)
                {
                    robin_hood::unordered_node_map<std::string, UniqueRef<Property>>::const_iterator it =
                        propertyMap.find(std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength()));
                    if (it != propertyMap.end() && it->second->GetFlags() & propertyFlags)
                    {
                        it->second->Set(scriptImpl.get(), Value::Create(propertyIt->value));
                        updateFlags |= it->second->GetFlags() & PropertyFlags::kPropertyFlag_InitiateUpdate;
                    }
                }

                return updateFlags;
            }
        }
    }
}