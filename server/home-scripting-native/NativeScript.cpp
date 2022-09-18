#include "NativeScript.hpp"
#include "NativeScriptSource.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScript::NativeScript(Ref<View> view, Ref<NativeScriptSource> scriptSource)
                : Script(view, scriptSource)
            {
            }
            NativeScript::~NativeScript()
            {
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

            Ref<Property> NativeScript::AddProperty(const std::string& name, Ref<Property> property)
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

            Ref<Method> NativeScript::AddMethod(const std::string& name, MethodCallback<> callback)
            {
                // Check existance
                if (!eventList.contains(name))
                {
                    // Create method instance
                    Ref<Method> e = Method::Create(name, callback);

                    // Add event to list
                    if (e != nullptr)
                    {
                        eventList[name] = e;
                        return e;
                    }
                }

                return nullptr;
            }
            bool NativeScript::RemoveMethod(const std::string& name)
            {
                return eventList.erase(name);
            }
            void NativeScript::ClearMethods()
            {
                eventList.clear();
            }

            bool NativeScript::Initialize()
            {
                bool result;

                // Initialize script
                {
                    result = Init();
                }

                // Take snapshot
                TakeSnapshot();

                return result;
            }

            bool NativeScript::Terminate()
            {
                return true;
            }
        }
    }
}