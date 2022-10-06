#include "NativeScript.hpp"
#include "NativeScriptSource.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScript::NativeScript(Ref<View> view, Ref<NativeScriptSource> scriptSource,
                                       UniqueRef<NativeScriptImpl> scriptImpl)
                : Script(view, scriptSource), scriptImpl(std::move(scriptImpl))
            {
            }
            NativeScript::~NativeScript()
            {
            }
            Ref<Script> NativeScript::Create(Ref<View> view, Ref<NativeScriptSource> scriptSource)
            {
                UniqueRef<NativeScriptImpl> scriptImpl = scriptSource->GetCreateCallback()();

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
                if (!attributeList.contains(name))
                {
                    // Parse attribute
                    rapidjson::Document document;

                    document.Parse(json);

                    // Add attribute to list
                    if (!document.HasParseError())
                    {
                        attributeList[name] = std::move(document);
                        return true;
                    }
                }

                return false;
            }
            bool NativeScript::RemoveAttribute(const std::string& name)
            {
                return attributeList.erase(name);
            }
            void NativeScript::ClearAttributes()
            {
                attributeList.clear();
            }

            Ref<Value> NativeScript::AddProperty(const std::string& name, Ref<Value> property)
            {
                // Check existance
                if (!propertyList.contains(name) && property != nullptr)
                {
                    propertyList[name] = property;
                    return property;
                }

                return nullptr;
            }
            bool NativeScript::RemoveProperty(const std::string& name)
            {
                return propertyList.erase(name);
            }
            void NativeScript::ClearProperties()
            {
                propertyList.clear();
            }

            bool NativeScript::AddMethod(const std::string& name, MethodCallback<> callback)
            {
                assert(callback != nullptr);

                // Check existance
                if (!methodList.contains(name))
                {
                    // Add method to list
                    methodList[name] = callback;
                    return true;
                }
                else
                    return false;
            }

            bool NativeScript::Invoke(const std::string& name, Ref<Value> parameter)
            {
                robin_hood::unordered_node_map<std::string, MethodCallback<>>::const_iterator it =
                    methodList.find(name);
                if (it != methodList.end())
                    return (scriptImpl.get()->*(it->second))(name, parameter);

                return false;
            }

            bool NativeScript::RemoveMethod(const std::string& name)
            {
                return methodList.erase(name);
            }
            void NativeScript::ClearMethods()
            {
                methodList.clear();
            }

            Ref<Event> NativeScript::AddEvent(const std::string& name)
            {
                // Check existance
                if (!eventList.contains(name))
                {
                    // Create event instance
                    Ref<Event> r = Event::Create();

                    // Add event to list
                    if (r != nullptr)
                    {
                        eventList[name] = r;
                        return r;
                    }
                }

                return nullptr;
            }
            bool NativeScript::RemoveEvent(const std::string& name)
            {
                return eventList.erase(name);
            }
            void NativeScript::ClearEvents()
            {
                eventList.clear();
            }
        }
    }
}