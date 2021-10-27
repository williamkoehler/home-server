#include "PluginManager.hpp"

namespace server
{
    void PluginManager::Load()
    {
        FILE *file = fopen("./plugins/plugins-info.json", "r");
        if (file == nullptr)
        {
            LOG_WARNING("Open/find 'plugins/plugins-info.json'");
            SaveDefault();
            throw std::runtime_error("Open/find file 'plugins/plugins-info.json'");
        }

        char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
        rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));

        rapidjson::Document document;
        if (document.ParseStream(stream).HasParseError() || !document.IsObject())
        {
            LOG_ERROR("Read 'plugins-info.json'");
            throw std::runtime_error("Read file 'plugins/plugins-info.json'");
        }

        //Load plugin
        rapidjson::Value::MemberIterator pluginListIt = document.FindMember("plugins");
        if (pluginListIt == document.MemberEnd() || !pluginListIt->value.IsArray())
        {
            LOG_ERROR("Missing 'plugins' in 'plugins/plugins-info.json'");
            throw std::runtime_error("Invalid file 'plugins/plugins-info.json'");
        }

        rapidjson::Value &pluginListJson = pluginListIt->value;
        for (rapidjson::Value::ValueIterator pluginIt = pluginListJson.Begin(); pluginIt != pluginListJson.End(); pluginIt++)
        {
            if (!pluginIt->IsString())
            {
                LOG_ERROR("Invalid plugin in 'plugins' in 'plugins/plugins-info.json'");
                continue;
            }

            LoadPlugin(std::string(pluginIt->GetString(), pluginIt->GetStringLength()));
        }

        fclose(file);
    }

    void PluginManager::SaveDefault()
    {
        LOG_INFO("Saving default plugin information to 'plugins/plugins-info.json'");

        //Create json
        rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

        document.AddMember("plugins", rapidjson::Value(rapidjson::kArrayType), allocator);

        //Save to file
        FILE *file = fopen("./plugins/plugins-info.json", "w");
        if (file == nullptr)
        {
            LOG_ERROR("Failing to open/find 'plugins/plugins-info.json'");
            throw std::runtime_error("Open/find file 'plugins/plugins-info.json'");
        }

        char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
        rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));

        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer = rapidjson::PrettyWriter<rapidjson::FileWriteStream>(stream);
        document.Accept(writer);

        fclose(file);
    }
}