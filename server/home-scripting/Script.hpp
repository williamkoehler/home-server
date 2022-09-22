#pragma once
#include "ScriptSource.hpp"
#include "View.hpp"
#include "common.hpp"
#include "utils/Method.hpp"
#include "utils/Value.hpp"

namespace server
{
    namespace scripting
    {
        class ScriptSource;

        class Value;
        class Method;
        class Event;

        class Script : public boost::enable_shared_from_this<Script>
        {
          protected:
            const Ref<View> view;
            const Ref<ScriptSource> scriptSource;

            robin_hood::unordered_node_map<std::string, rapidjson::Document> attributeList;
            robin_hood::unordered_node_map<std::string, Ref<Value>> propertyList;
            robin_hood::unordered_node_map<std::string, Ref<Method>> methodList;
            robin_hood::unordered_node_map<std::string, Ref<Event>> eventList;

          public:
            Script(Ref<View> view, Ref<ScriptSource> scriptSource);
            virtual ~Script();

            inline Ref<View> GetView() const
            {
                return view;
            }

            /// @brief Get script source id
            ///
            /// @return Script source id
            identifier_t GetSourceID() const
            {
                return scriptSource->GetID();
            }

            Ref<Value> GetProperty(const std::string& id);
            Ref<Method> GetMethod(const std::string& id);
            Ref<Event> GetEvent(const std::string& id);

            /// @brief Initialize script (must be called by host thread)
            ///
            /// @return Successful
            virtual bool Initialize() = 0;

            /// @brief Terminate script (must be called by host thread)
            ///
            /// @return Successful
            virtual bool Terminate() = 0;

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSetState(rapidjson::Value& input);
        };
    }
}