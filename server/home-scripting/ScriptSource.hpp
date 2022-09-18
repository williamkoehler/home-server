#pragma once
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        class View;

        enum class ScriptUsage
        {
            kUnknownUsage,
            kDeviceScriptUsage,
        };

        std::string StringifyScriptUsage(ScriptUsage usage);
        ScriptUsage ParseScriptUsage(const std::string& usage);

        enum class ScriptLanguage
        {
            kUnknownScriptLanguage,
            kNativeScriptLanguage,
            kJSScriptLanguage
        };

        std::string StringifyScriptLanguage(ScriptLanguage language);
        ScriptLanguage ParseScriptLanguage(const std::string& language);

        class ScriptSource : public boost::enable_shared_from_this<ScriptSource>
        {
          protected:
            const identifier_t id;

            std::string name;

            const ScriptUsage usage;

            std::string content;

            /// @brief Checksum (changes when the data changes)
            boost::atomic_uint64_t checksum;

            inline void UpdateChecksum()
            {
                checksum = XXH64(content.data(), content.size(), 0x323435367A683567);
            }

          public:
            ScriptSource(identifier_t id, const std::string& name, ScriptUsage usage, const std::string_view& content);
            virtual ~ScriptSource();

            inline identifier_t GetID() const
            {
                return id;
            }

            std::string GetName();
            bool SetName(const std::string& v);

            inline ScriptUsage GetUsage() const
            {
                return usage;
            }

            virtual ScriptLanguage GetLanguage() = 0;

            std::string GetContent();

            /// @brief Set content / source code
            ///
            /// @param v Source code
            /// @return Successfulness
            virtual bool SetContent(const std::string_view& v);

            inline uint64_t GetChecksum()
            {
                return checksum;
            }

            /// @brief Create script from script source
            /// 
            /// @param view Sender view
            /// @return Script
            virtual Ref<Script> CreateScript(Ref<View> view) = 0;

            void JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSet(rapidjson::Value& input);

            void JsonGetContent(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator);
            void JsonSetContent(rapidjson::Value& input);
        };
    }
}