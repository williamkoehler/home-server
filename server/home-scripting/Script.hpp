#pragma once
#include "PropertyFlags.hpp"
#include "ScriptSource.hpp"
#include "common.hpp"
#include "interface/Event.hpp"
#include "Value.hpp"
#include "view/View.hpp"

namespace server
{
    namespace scripting
    {
        class ScriptSource;

        class Value;

        class Script : public boost::enable_shared_from_this<Script>
        {
          protected:
            const Ref<View> view;
            const Ref<ScriptSource> scriptSource;

            /// @brief Script attributes
            ///
            robin_hood::unordered_node_map<std::string, rapidjson::Document> attributeMap;

            /// @brief Script events
            ///
            robin_hood::unordered_node_map<std::string, Event> eventMap;

          public:
            Script(const Ref<View>& view, const Ref<ScriptSource>& scriptSource);
            virtual ~Script();

            /// @brief Get view to parent object
            ///
            /// @return Ref<View> View
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

            /// @brief Get preferred lazy update interval
            ///
            /// @return size_t Lazy update interval (in seconds)
            size_t GetLazyUpdateInterval() const;

            /// @brief Get preferred update interval
            ///
            /// @return size_t Update interval (in seconds)
            size_t GetUpdateInterval() const;

            /// @brief Initialize script (ONLY CALL ONCE AFTER CREATION)
            ///
            /// @return Successfulness
            virtual bool Initialize();

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
            /// @return Successfulness
            bool LazyUpdate();

            /// @brief Post update script
            /// @note Updates internal script state and properties
            ///
            void PostLazyUpdate();

            /// @brief Update script
            /// @note Updates internal script state and properties
            ///
            /// @return Successfulness
            bool Update();

            /// @brief Post update script
            /// @note Updates internal script state and properties
            ///
            void PostUpdate();

            /// @brief Bind view method to event
            ///
            /// @param event Event name
            /// @param view Invokable view
            /// @param method Method name
            /// @return EventConnection Event connection
            EventConnection Bind(const std::string& event, const Ref<View>& view, const std::string& method);

            void JsonGetAttributes(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);

            virtual void JsonGetProperties(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                                           PropertyFlags flags = kPropertyFlag_Visible) = 0;
            virtual PropertyFlags JsonSetProperties(const rapidjson::Value& input,
                                                    PropertyFlags flags = kPropertyFlag_All) = 0;
        };
    }
}