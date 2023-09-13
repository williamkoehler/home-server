#pragma once
#include "common.hpp"
#include "value.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        class Scriptable
        {
          protected:
            Ref<Script> script;

          public:
            /// @brief Get script
            ///
            /// @return Ref<scripting::Script> Script or null
            inline Ref<scripting::Script> GetScript() const
            {
                return script;
            }

            /// @brief Get script source id
            ///
            /// @return identifier_t Script source id
            identifier_t GetScriptSourceId() const;

            /// @brief Set script
            ///
            /// @param scriptSourceId Script source id
            void SetScript(identifier_t scriptSourceId);

            /// @brief Get view
            ///
            /// @return Ref<scripting::View> Get view of this object
            virtual Ref<scripting::View> GetView() = 0;

            /// @brief Invoke script method
            ///
            /// @param event Method name
            void Invoke(const std::string& method, const Value& parameter);

            void JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const;
            bool JsonSetState(const rapidjson::Value& input);
        };
    }
}