#include "ScriptSource.hpp"
#include "Script.hpp"
#include <home-database/Database.hpp>

namespace server
{
    namespace scripting
    {
        std::string StringifyScriptUsage(ScriptUsage usage)
        {
            switch (usage)
            {
            case ScriptUsage::kDeviceScriptUsage:
                return "device";
            case ScriptUsage::kServiceScriptUsage:
                return "service";
            default:
                return "unknown";
            }
        }
        ScriptUsage ParseScriptUsage(const std::string& usage)
        {
            switch (crc32(usage.data(), usage.size()))
            {
            case CRC32("device"):
                return ScriptUsage::kDeviceScriptUsage;
            case CRC32("service"):
                return ScriptUsage::kServiceScriptUsage;
            default:
                return ScriptUsage::kUnknownUsage;
            }
        }

        std::string StringifyScriptLanguage(ScriptLanguage language)
        {
            switch (language)
            {
            case ScriptLanguage::kNativeScriptLanguage:
                return "native";
            case ScriptLanguage::kJSScriptLanguage:
                return "javascript";
            default:
                return "unknown";
            }
        }
        ScriptLanguage ParseScriptLanguage(const std::string& language)
        {
            switch (crc32(language.data(), language.size()))
            {
            case CRC32("native"):
                return ScriptLanguage::kNativeScriptLanguage;
            case CRC32("javascript"):
                return ScriptLanguage::kJSScriptLanguage;
            default:
                return ScriptLanguage::kUnknownScriptLanguage;
            }
        }

        ScriptSource::ScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                                   const std::string_view& content)
            : id(id), name(name), usage(usage), content(content)
        {
            UpdateChecksum();
        }
        ScriptSource::~ScriptSource()
        {
        }

        std::string ScriptSource::GetName()
        {
            return name;
        }
        bool ScriptSource::SetName(const std::string& v)
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            if (database->UpdateScriptSourcePropName(id, name, v))
            {
                name = v;

                return true;
            }
            else
                return false;
        }

        std::string ScriptSource::GetContent()
        {
            return content;
        }
        bool ScriptSource::SetContent(const std::string_view& v)
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Update database
            if (database->UpdateScriptSourcePropContent(id, v))
            {
                content = v;

                return true;
            }

            return false;
        }

        void ScriptSource::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.AddMember("id", rapidjson::Value(id), allocator);

            std::string languageStr = StringifyScriptLanguage(GetLanguage());
            output.AddMember("language", rapidjson::Value(languageStr.data(), languageStr.size(), allocator),
                             allocator);

            output.AddMember("name", rapidjson::Value(name.c_str(), name.size(), allocator), allocator);

            std::string usageStr = StringifyScriptUsage(usage);
            output.AddMember("usage", rapidjson::Value(usageStr.data(), usageStr.size(), allocator), allocator);
        }
        void ScriptSource::JsonSet(rapidjson::Value& input)
        {
            assert(input.IsObject());

            // Decode properties
            rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
            if (nameIt != input.MemberEnd() && nameIt->value.IsString())
                SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));
        }

        void ScriptSource::JsonGetContent(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.AddMember("content", rapidjson::Value((const char*)content.data(), content.size(), allocator),
                             allocator);
        }
        void ScriptSource::JsonSetContent(rapidjson::Value& input)
        {
            assert(input.IsObject());

            // Decode properties
            rapidjson::Value::MemberIterator contentIt = input.FindMember("content");
            if (contentIt != input.MemberEnd() && contentIt->value.IsString())
                SetContent(
                    std::string_view((const char*)contentIt->value.GetString(), contentIt->value.GetStringLength()));
        }
    }
}