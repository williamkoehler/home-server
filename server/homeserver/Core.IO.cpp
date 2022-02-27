#include "Core.hpp"
#include "tools/EMail.hpp"
#include "Version.hpp"

namespace server
{
    void Core::Load()
    {
        LOG_INFO("Loading core information from 'core-info.json'");

        FILE *file = fopen("core-info.json", "r");
        if (file == nullptr)
        {
            LOG_ERROR("Failing to open/find 'core-info.json'");
            if (!boost::filesystem::exists("core-info.json"))
                SaveDefault();
            throw std::runtime_error("Open/find file 'core-info.json'");
        }

        boost::lock_guard lock(mutex);

        char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
        rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));

        rapidjson::Document document;
        if (document.ParseStream(stream).HasParseError() || !document.IsObject())
        {
            LOG_ERROR("Failing to read 'core-info.json'");
            throw std::runtime_error("Read file 'core-info.json'");
        }

        // Load server name
        rapidjson::Value::MemberIterator nameIt = document.FindMember("name");
        if (nameIt != document.MemberEnd() && nameIt->value.IsString())
            name.assign(nameIt->value.GetString(), nameIt->value.GetStringLength());
        else
        {
            name = "My Home Automation server v" SERVER_VERSION;
            LOG_WARNING("Missing 'name' in 'core-info.json'. Name will be set to '{0}'.", name);
        }

        // Load web server address
        rapidjson::Value::MemberIterator addressIt = document.FindMember("address");
        if (addressIt != document.MemberEnd() && addressIt->value.IsString())
            address = std::string(addressIt->value.GetString(), addressIt->value.GetStringLength());
        else
        {
            address = "0.0.0.0";
            LOG_WARNING("Missing 'address' in 'core-info.json'. Address will be set to default '0.0.0.0'.");
        }

        // Load web server port
        rapidjson::Value::MemberIterator portIt = document.FindMember("port");
        if (portIt != document.MemberEnd() && portIt->value.IsUint())
            port = portIt->value.GetUint();
        else
        {
            port = 443;
            LOG_WARNING("Missing 'port' in 'core-info.json'. Port will be set to default '{0}'.", port);
        }

        LOG_INFO("Server address is {0}:{1}", address, port);

        // Load web server external address
        rapidjson::Value::MemberIterator externalAddressIt = document.FindMember("external-address");
        if (externalAddressIt != document.MemberEnd() && externalAddressIt->value.IsString())
            externalAddress = std::string(externalAddressIt->value.GetString(), externalAddressIt->value.GetStringLength());
        else
        {
            externalAddress = "";
            LOG_WARNING("Missing 'external-address' in 'core-info.json'.");
        }

        // Load web server external address
        rapidjson::Value::MemberIterator externalPortIt = document.FindMember("external-port");
        if (externalPortIt != document.MemberEnd() && externalPortIt->value.IsUint())
            externalPort = externalPortIt->value.GetUint();
        else
        {
            externalPort = port;
            LOG_WARNING("Missing 'external-port' in 'core-info.json'.");
        }

        // Load thread counts
        rapidjson::Value::MemberIterator additionalThreadCountIt = document.FindMember("additional-thread-count");
        if (additionalThreadCountIt != document.MemberEnd() && additionalThreadCountIt->value.IsUint())
            threadCount = std::max(additionalThreadCountIt->value.GetUint() + 1, 1u);
        else
        {
            threadCount = boost::thread::hardware_concurrency() - 1;
            LOG_WARNING("Missing 'main-thread-count' in 'core-info.json'. Default value will be used.");
        }

        if (threadCount == 1)
        {
            LOG_WARNING("No additional worker thread is used. This can cause some slowdowns.");
        }

        LOG_INFO("Using {0} main thread(s).", threadCount);

        // Load EMail
        rapidjson::Value::MemberIterator emailIt = document.FindMember("email-service");
        if (emailIt != document.MemberEnd() && emailIt->value.IsObject())
        {
            rapidjson::Value::MemberIterator addressIt = emailIt->value.FindMember("server-address");
            rapidjson::Value::MemberIterator portIt = emailIt->value.FindMember("server-port");
            rapidjson::Value::MemberIterator userAddressIt = emailIt->value.FindMember("user-address");
            rapidjson::Value::MemberIterator userPasswordIt = emailIt->value.FindMember("user-password");
            rapidjson::Value::MemberIterator recipientListIt = emailIt->value.FindMember("recipients");

            if (addressIt != document.MemberEnd() && addressIt->value.IsString() &&
                portIt != document.MemberEnd() && portIt->value.IsUint() &&
                userAddressIt != document.MemberEnd() && userAddressIt->value.IsString() &&
                userPasswordIt != document.MemberEnd() && userPasswordIt->value.IsString() &&
                recipientListIt != document.MemberEnd() && recipientListIt->value.IsArray())
            {
                Ref<EMail> email = EMail::Create(addressIt->value.GetString(), portIt->value.GetUint(),
                                                 userAddressIt->value.GetString(), userPasswordIt->value.GetString());

                rapidjson::Value &recipientListJson = recipientListIt->value;
                for (rapidjson::Value::ValueIterator recipientIt = recipientListJson.Begin(); recipientIt != recipientListJson.End(); recipientIt++)
                    if (recipientIt->IsString())
                        email->AddRecipient(recipientIt->GetString());

                rapidjson::Value::MemberIterator sendEMailAtLaunchIt = emailIt->value.FindMember("send-email-at-launch");
                if (sendEMailAtLaunchIt != emailIt->value.MemberEnd() && sendEMailAtLaunchIt->value.IsBool())
                    if (sendEMailAtLaunchIt->value.GetBool())
                        email->Send("HomeAutomation Server", "Hello, \n\nThe home automation server has just started!\n\nNote that this email is sent automatically as soon as the server is started!\nYou can disable this feature in 'core-info.json'.");
            }
        }

        fclose(file);
    }
    void Core::SaveDefault()
    {
        boost::shared_lock_guard lock(mutex);

        LOG_INFO("Saving default core information to 'core-info.json'");

        // Create json
        rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

        rapidjson::Document::AllocatorType &allocator = document.GetAllocator();

        document.AddMember("name", rapidjson::Value("Home Automation Server", 22), allocator);
        document.AddMember("port", rapidjson::Value(443), allocator);
        document.AddMember("address", rapidjson::Value("0.0.0.0", 7), allocator);
        {
            const size_t maxThreadCount = std::clamp(boost::thread::hardware_concurrency(), static_cast<uint32_t>(1), static_cast<uint32_t>(11));
            document.AddMember("additional-thread-count", rapidjson::Value(maxThreadCount - 1), allocator);
        }
        document.AddMember("use-ssh", rapidjson::Value(false), allocator);

        {
            rapidjson::Value emailServiceJson = rapidjson::Value(rapidjson::kObjectType);

            emailServiceJson.AddMember("server-address", rapidjson::Value("server address"), allocator);
            emailServiceJson.AddMember("server-port", rapidjson::Value("465"), allocator);
            emailServiceJson.AddMember("user-address", rapidjson::Value("email.address@domain"), allocator);
            emailServiceJson.AddMember("user-password", rapidjson::Value("password"), allocator);
            emailServiceJson.AddMember("recipients", rapidjson::Value(rapidjson::kArrayType), allocator);

            document.AddMember("email-service", emailServiceJson, allocator);
        }

        // Save to file
        FILE *file = fopen("core-info.json", "w");
        if (file == nullptr)
        {
            LOG_ERROR("Failing to open/find 'core-info.json'");
            throw std::runtime_error("Open/find file 'core-info.json'");
        }

        char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
        rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));

        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer = rapidjson::PrettyWriter<rapidjson::FileWriteStream>(stream);
        document.Accept(writer);

        fclose(file);
    }
}