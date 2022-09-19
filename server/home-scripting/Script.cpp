#include "Script.hpp"
#include "utils/Method.hpp"
#include "utils/Value.hpp"

namespace server
{
    namespace scripting
    {
        Script::Script(Ref<View> view, Ref<ScriptSource> scriptSource) : view(view), scriptSource(scriptSource)
        {
            assert(view != nullptr);
            assert(scriptSource != nullptr);
        }
        Script::~Script()
        {
        }

        Ref<Value> Script::GetProperty(const std::string& id)
        {
            const robin_hood::unordered_node_map<std::string, Ref<Value>>::const_iterator it = propertyList.find(id);
            if (it == propertyList.end())
                return nullptr;

            return it->second;
        }

        Ref<Method> Script::GetMethod(const std::string& id)
        {
            const robin_hood::unordered_node_map<std::string, Ref<Method>>::const_iterator it = methodList.find(id);
            if (it == methodList.end())
                return nullptr;

            return it->second;
        }

        Ref<Event> Script::GetEvent(const std::string& id)
        {
            const robin_hood::unordered_node_map<std::string, Ref<Event>>::const_iterator it = eventList.find(id);
            if (it == eventList.end())
                return nullptr;

            return it->second;
        }

        void Script::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Build script attributes
            rapidjson::Value attributesJson = rapidjson::Value(rapidjson::kObjectType);
            attributesJson.MemberReserve(attributeList.size(), allocator);

            for (auto& [id, attribute] : attributeList)
                attributesJson.AddMember(rapidjson::Value(id.data(), id.size(), allocator),
                                         rapidjson::Value(attribute, allocator, true), allocator);

            output.AddMember("attributes", attributesJson, allocator);
        }
        void Script::JsonSet(rapidjson::Value& input)
        {
            throw std::exception();
        }

        void Script::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

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
            assert(input.IsObject());

            for (rapidjson::Value::MemberIterator propertyIt = input.MemberBegin(); propertyIt != input.MemberEnd();
                 propertyIt++)
            {
                Ref<Value> property =
                    GetProperty(std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength()));

                if(property != nullptr)
                    property->JsonSet(propertyIt->value);
            }
        }
    }
}