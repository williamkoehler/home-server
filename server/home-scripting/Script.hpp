#pragma once
#include "PropertyFlags.hpp"
#include "ScriptSource.hpp"
#include "View.hpp"
#include "common.hpp"
#include "tasks/Task.hpp"
#include "utils/Event.hpp"
#include "utils/Value.hpp"

namespace server
{
    namespace scripting
    {
        class ScriptSource;

        class Value;

        class Task;

        class Script : public boost::enable_shared_from_this<Script>
        {
          protected:
            friend class Task;

            const Ref<View> view;
            const Ref<ScriptSource> scriptSource;

            size_t lastUpdateTime, updateInterval;

            /// @brief Script attributes
            ///
            robin_hood::unordered_node_map<std::string, rapidjson::Document> attributeMap;

            /// @brief Script events
            ///
            robin_hood::unordered_node_map<std::string, Event> eventMap;

            /// @brief Script tasks
            ///
            boost::container::vector<WeakRef<Task>> taskMap;

          public:
            Script(const Ref<View>& view, const Ref<ScriptSource>& scriptSource);
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

            /// @brief Get property value
            ///
            /// @param name Property name
            /// @return Value
            virtual Value GetProperty(const std::string& name) = 0;

            /// @brief Set property value
            ///
            /// @param name Property name
            /// @param value Value
            virtual void SetProperty(const std::string& name, const Value& value) = 0;

            /// @brief Invoke method
            ///
            /// @param name Method name
            /// @param parameter Parameter
            /// @return Successfulness
            virtual bool Invoke(const std::string& name, const Value& parameter) = 0;

            /// @brief Post invoke method
            ///
            /// @param name Method name
            /// @param parameter Parameter
            void PostInvoke(const std::string& name, const Value& parameter);

            /// @brief Update script (lazy update)
            /// @note Updates internal script state and properties
            ///
            /// @param minUpdateInterval Min update interval (seconds)
            /// @return Successfulness
            virtual bool Update(size_t minUpdateInterval);

            /// @brief Post update script (lazy update)
            /// @note Updates internal script state and properties
            ///
            /// @param minUpdateInterval Min update interval (seconds)
            void PostUpdate(size_t minUpdateInterval);

            /// @brief Bind view method to event
            ///
            /// @param event Event name
            /// @param view Invokable view
            /// @param method Method name
            /// @return EventConnection Event connection
            EventConnection Bind(const std::string& event, const Ref<View>& view, const std::string& method);

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

            virtual void JsonGetProperties(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                                           uint8_t propertyFlags = PropertyFlags::kPropertyFlag_Visible) = 0;
            virtual uint8_t JsonSetProperties(const rapidjson::Value& input,
                                           uint8_t propertyFlags = PropertyFlags::kPropertyFlags_All) = 0;
        };
    }
}