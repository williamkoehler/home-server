#include "Script.hpp"

namespace server
{
    namespace scripting
    {
        Script::Script(Ref<View> view, Ref<ScriptSource> scriptSource)
            : view(view), scriptSource(scriptSource)
        {
            assert(view != nullptr);
            assert(scriptSource != nullptr);
        }
        Script::~Script()
        {
        }

        Ref<Property> Script::GetProperty(const std::string& id)
        {
            // Lock main mutex
            boost::lock_guard lock(mutex);

            const robin_hood::unordered_node_map<std::string, Ref<Property>>::const_iterator it = propertyList.find(id);
            if (it == propertyList.end())
                return nullptr;

            return it->second;
        }

        bool Script::Invoke(const std::string& event)
        {
            // Lock main mutex
            boost::lock_guard lock(mutex);

            const robin_hood::unordered_node_map<std::string, Ref<Event>>::const_iterator it = eventList.find(event);
            if (it != eventList.end())
            {
                // Invoke event
                it->second->PostInvoke();

                return true;
            }

            return false;
        }

        void Script::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock main mutex
            boost::lock_guard lock(mutex);

            // Reserve memory
            output.MemberReserve(attributeList.size(), allocator);
            for (auto& [id, attribute] : attributeList)
                output.AddMember(rapidjson::Value(id.data(), id.size(), allocator),
                                 rapidjson::Value(attribute, allocator, true), allocator);
        }
        void Script::JsonSet(rapidjson::Value& input)
        {
            throw std::exception();
        }

        void Script::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock main mutex
            boost::lock_guard lock(mutex);

            output.MemberReserve(propertyList.size(), allocator);
            for (auto& [id, property] : propertyList)
            {
                // Add property
                output.AddMember(rapidjson::Value(id.data(), id.size(), allocator), property->JsonGet(allocator),
                                 allocator);
            }
        }
        void Script::JsonSetState(rapidjson::Value& input)
        {
            LOG_CODE_MISSING("Script json set state");
        }
    }
}