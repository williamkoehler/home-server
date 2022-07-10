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

            bool NativeScript::AddAttribute(const std::string& id, const char* json)
            {
                // Check existance
                if (!attributeList.contains(id))
                {
                    // Parse attribute
                    rapidjson::Document document;

                    document.Parse(json);

                    // Add attribute to list
                    if (!document.HasParseError())
                    {
                        attributeList[id] = std::move(document);
                        return true;
                    }
                }

                return false;
            }
            bool NativeScript::RemoveAttribute(const std::string& id)
            {
                return attributeList.erase(id);
            }
            void NativeScript::ClearAttributes()
            {
                attributeList.clear();
            }

            Ref<Property> NativeScript::AddProperty(const std::string& id, Ref<Property> property)
            {
                // Check existance
                if (!propertyList.contains(id) && property != nullptr)
                {
                    propertyList[id] = property;
                    return property;
                }

                return nullptr;
            }
            bool NativeScript::RemoveProperty(const std::string& id)
            {
                return propertyList.erase(id);
            }
            void NativeScript::ClearProperties()
            {
                propertyList.clear();
            }

            Ref<Event> NativeScript::AddEvent(const std::string& id, EventMethod<> event)
            {
                // Check existance
                if (!eventList.contains(id))
                {
                    // Create event instance
                    Ref<Event> e = boost::make_shared<Event>(id, event);

                    // Add event to list
                    if (e != nullptr)
                    {
                        eventList[id] = e;
                        return e;
                    }
                }

                return nullptr;
            }
            bool NativeScript::RemoveEvent(const std::string& id)
            {
                return eventList.erase(id);
            }
            void NativeScript::ClearEvents()
            {
                eventList.clear();
            }

            bool NativeScript::Initialize()
            {
                bool result;

                // Terminate script
                {
                    boost::lock_guard lock(mutex);

                    result = Init();
                }

                // Take snapshot
                TakeSnapshot();

                return result;
            }
        }
    }
}