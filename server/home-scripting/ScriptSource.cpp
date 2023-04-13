#include "ScriptSource.hpp"
#include "Script.hpp"
#include <home-database/Database.hpp>

namespace server
{
    namespace scripting
    {
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

        ScriptSource::ScriptSource(identifier_t id, const std::string& name, const std::string_view& content)
            : id(id), name(name), content(content), updateNeeded(false)
        {
            UpdateChecksum();
        }
        ScriptSource::~ScriptSource()
        {
        }

        bool ScriptSource::Save() const
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Generate json additional config
            rapidjson::StringBuffer config = rapidjson::StringBuffer();
            {
                // Generate json
                rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);
                JsonGetConfig(document, document.GetAllocator());

                // Stringify json
                rapidjson::Writer<rapidjson::StringBuffer> writer = rapidjson::Writer<rapidjson::StringBuffer>(config);
                document.Accept(writer);
            }

            // Update database
            return database->UpdateScriptSource(id, name, std::string_view(config.GetString(), config.GetSize()));
        }

        bool ScriptSource::SaveContent() const
        {
            Ref<Database> database = Database::GetInstance();
            assert(database != nullptr);

            // Update database
            return database->UpdateScriptSourceContent(id, std::string_view(content.data(), content.size()));
        }

        void ScriptSource::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            assert(output.IsObject());

            output.AddMember("id", rapidjson::Value(id), allocator);

            std::string languageStr = StringifyScriptLanguage(GetLanguage());
            output.AddMember("language", rapidjson::Value(languageStr.data(), languageStr.size(), allocator),
                             allocator);

            output.AddMember("name", rapidjson::Value(name.c_str(), name.size(), allocator), allocator);

            JsonGetConfig(output, allocator);
        }
        bool ScriptSource::JsonSet(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            bool update = JsonSetConfig(input);

            rapidjson::Value::ConstMemberIterator nameIt = input.FindMember("name");
            if (nameIt != input.MemberEnd() && nameIt->value.IsString())
            {
                name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
                update = true;
            }

            return update;
        }

        void ScriptSource::JsonGetContent(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
        {
            assert(output.IsObject());

            output.AddMember("content", rapidjson::Value((const char*)content.data(), content.size(), allocator),
                             allocator);
        }
        bool ScriptSource::JsonSetContent(const rapidjson::Value& input)
        {
            assert(input.IsObject());

            bool update = false;

            rapidjson::Value::ConstMemberIterator contentIt = input.FindMember("content");
            if (contentIt != input.MemberEnd() && contentIt->value.IsString())
            {
                SetContent(
                    std::string_view((const char*)contentIt->value.GetString(), contentIt->value.GetStringLength()));
                update = true;
            }

            return update;
        }

        void ScriptSource::ApiGet(const ApiRequestMessage& request, ApiResponseMessage& response, const Ref<ApiSession>& session) const
        {
            (void)request;
            (void)session;

            JsonGet(response.GetJsonDocument(), response.GetJsonAllocator());
        }
        bool ScriptSource::ApiSet(const ApiRequestMessage& request, ApiResponseMessage& response, const Ref<ApiSession>& session)
        {
            (void)response;
            (void)session;

            bool update = JsonSet(request.GetJsonDocument());

            if (update)
                Save();

            return update;
        }

        void ScriptSource::ApiGetContent(const ApiRequestMessage& request, ApiResponseMessage& response,
                                         const Ref<ApiSession>& session) const
        {
            (void)request;
            (void)session;

            JsonGetContent(response.GetJsonDocument(), response.GetJsonAllocator());
        }
        bool ScriptSource::ApiSetContent(const ApiRequestMessage& request, ApiResponseMessage& response,
                                         const Ref<ApiSession>& session)
        {
            (void)response;
            (void)session;

            bool update = JsonSetContent(request.GetJsonDocument());

            if (update)
                SaveContent();

            return update;
        }
    }
}