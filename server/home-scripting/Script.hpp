#pragma once
#include "ScriptSource.hpp"
#include "View.hpp"
#include "common.hpp"
#include <home-threading/Worker.hpp>

namespace server
{
    namespace scripting
    {
        class ScriptSource;

        class Property;
        class Event;
        class Timer;

        class Script : public boost::enable_shared_from_this<Script>
        {
          protected:
            boost::mutex mutex;

            const Ref<View> view;
            const Ref<ScriptSource> scriptSource;

            robin_hood::unordered_node_map<std::string, rapidjson::Document> attributeList;
            robin_hood::unordered_node_map<std::string, Ref<Property>> propertyList;
            robin_hood::unordered_node_map<std::string, Ref<Event>> eventList;
            robin_hood::unordered_node_map<std::string, Ref<Timer>> timerList;

            boost::mutex snapshotMutex;
            rapidjson::Document snapshot;

          public:
            Script(Ref<View> view, Ref<ScriptSource> scriptSource);
            virtual ~Script();

            inline Ref<threading::Worker> GetWorker() const
            {
                return view->GetWorker();
            }

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

            Ref<Property> GetProperty(const std::string& id);
            Ref<Event> GetEvent(const std::string& id);
            Ref<Timer> GetTimer(const std::string& id);

            /// @brief Initialize script (must be called by host thread)
            ///
            /// @return Successful
            virtual bool Initialize() = 0;

            /// @brief Call script event
            ///
            /// @param event Event name
            /// @return Successfulness
            virtual bool Invoke(const std::string& event) = 0;

            /// @brief Let home worker call script event
            ///
            /// @param event Event name
            /// @return Successfulness
            bool PostInvoke(const std::string& event);

            /// @brief Terminate script (must be called by host thread)
            ///
            /// @return Successful
            virtual bool Terminate() = 0;

            /// @brief Take property snapshot
            void TakeSnapshot();

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSetState(rapidjson::Value& input);
        };
    }
}