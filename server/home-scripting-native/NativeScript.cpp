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

            Ref<Event> NativeScript::AddEvent(const std::string& id, EventCallback callback)
            {
                // Check existance
                if (!eventList.contains(id))
                {
                    // Create event instance
                    Ref<NativeEvent> event = boost::make_shared<NativeEvent>(shared_from_this(), callback);

                    // Add event to list
                    if (event != nullptr)
                    {
                        eventList[id] = event;
                        return event;
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

            Ref<Timer> NativeScript::AddTimer(const std::string& id, const std::string& event)
            {
                // Check existance
                if (!timerList.contains(id))
                {
                    // Create timer instance
                    Ref<Timer> timer = boost::make_shared<Timer>(shared_from_this(), event);

                    // Add timer to list
                    if (timer != nullptr)
                    {
                        timerList[id] = timer;
                        return timer;
                    }
                }

                return nullptr;
            }
            bool NativeScript::RemoveTimer(const std::string& id)
            {
                return eventList.erase(id);
            }
            void NativeScript::ClearTimers()
            {
                eventList.clear();
            }

            bool NativeScript::Initialize()
            {
                bool result;

                // Terminate script
                {
                    boost::lock_guard lock(mutex);

                    result = InitializeScript();
                }

                // Take snapshot
                TakeSnapshot();

                return result;
            }

            bool NativeScript::Invoke(const std::string& event)
            {

                Ref<NativeEvent> r = boost::dynamic_pointer_cast<NativeEvent>(GetEvent(event));
                if (r != nullptr)
                {
                    try
                    {
                        // Call C++ Method
                        ((Script*)this->*r->GetCallback())();
                    }
                    catch (std::exception)
                    {
                    }

                    // Take snapshot
                    TakeSnapshot();

                    return true;
                }

                return false;
            }

            bool NativeScript::Terminate()
            {
                bool result;

                // Terminate script
                {
                    boost::lock_guard lock(mutex);

                    result = TerminateScript();
                }

                // Take snapshot
                TakeSnapshot();

                return result;
            }
        }
    }
}