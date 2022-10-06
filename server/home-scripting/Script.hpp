#pragma once
#include "ScriptSource.hpp"
#include "View.hpp"
#include "common.hpp"
#include "tasks/Task.hpp"
#include "utils/Value.hpp"

namespace server
{
    namespace scripting
    {
        class ScriptSource;

        class Value;
        class Event;

        class Task;

        class Script : public boost::enable_shared_from_this<Script>
        {
          protected:
            friend class Task;

            const Ref<View> view;
            const Ref<ScriptSource> scriptSource;

            robin_hood::unordered_node_map<std::string, rapidjson::Document> attributeList;
            robin_hood::unordered_node_map<std::string, Ref<Value>> propertyList;
            robin_hood::unordered_node_map<std::string, Ref<Event>> eventList;

            boost::container::vector<WeakRef<Task>> taskList;

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
            Ref<Event> GetEvent(const std::string& id);

            /// @brief Initialize script (ONLY CALL ONCE AFTER CREATION)
            ///
            /// @return Successfulness
            virtual bool Initialize();

            /// @brief Add timer task
            ///
            /// @param method Method name
            /// @param interval Interval
            void AddTimerTask(const std::string& method, size_t interval);

            /// @brief Remove finished tasks
            /// @note This method is automatically called when a task finishes
            ///
            void CleanTasks();

            /// @brief Invoke method
            ///
            /// @param name Method name
            /// @param parameter Parameter
            /// @return Successfulness
            virtual bool Invoke(const std::string& name, Ref<Value> parameter) = 0;

            /// @brief Post invoke method
            ///
            /// @param name Method name
            /// @param parameter Parameter
            void PostInvoke(const std::string& name, Ref<Value> parameter);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSetState(rapidjson::Value& input);
        };
    }
}