#include "ScriptManager.hpp"
#include "ScriptSource.hpp"

namespace scripting
{
	#define SCRIBBLE_INFO "Script-info.json"

	void ScriptManager::Load()
	{
		LOG_INFO("Loading script information from 'script-info.json'");

		FILE* file = fopen("script-info.json", "r");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find 'script-info.json'");
			throw std::runtime_error("Open/find file 'script-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));

		rapidjson::Document document;
		if (document.ParseStream(stream).HasParseError() || !document.IsObject())
		{
			LOG_ERROR("Read 'script-info.json'");
			throw std::runtime_error("Read file 'script-info.json'");
		}

		//Read script list
		{
			rapidjson::Value::MemberIterator scriptListIt = document.FindMember("script-list");
			if (scriptListIt != document.MemberEnd() && scriptListIt->value.IsArray())
			{
				rapidjson::Value& scriptListJson = scriptListIt->value;
				for (rapidjson::Value::ValueIterator scriptIt = scriptListJson.Begin(); scriptIt != scriptListJson.End(); scriptIt++)
				{
					if (!scriptIt->IsObject())
					{
						LOG_ERROR("Invalid script in 'script-list' in 'script-info.json'");
						throw std::runtime_error("Invalid file 'home-info.json'");
					}

					//Read name
					rapidjson::Value::MemberIterator nameIt = scriptIt->FindMember("name");
					if (nameIt == scriptIt->MemberEnd() || !nameIt->value.IsString())
					{
						LOG_ERROR("Missing 'name' in device in 'script-list' in 'script-info.json'");
						throw std::runtime_error("Invalid file 'script-info.json'");
					}

					//Read id
					rapidjson::Value::MemberIterator idIt = scriptIt->FindMember("id");
					if (idIt == scriptIt->MemberEnd() || !idIt->value.IsUint())
					{
						LOG_ERROR("Missing 'id' in script in 'script-list' in 'script-info.json'");
						throw std::runtime_error("Invalid file 'script-info.json'");
					}

					//Read language
					rapidjson::Value::MemberIterator languageIt = scriptIt->FindMember("language");
					if (languageIt == scriptIt->MemberEnd() || !languageIt->value.IsString())
					{
						LOG_ERROR("Missing 'language' in script in 'script-list' in 'script-info.json'");
						throw std::runtime_error("Invalid file 'script-info.json'");
					}

					std::string name = std::string(nameIt->value.GetString(), nameIt->value.GetStringLength());

					// Get language
					std::string lang = std::string(languageIt->value.GetString(), languageIt->value.GetStringLength());
					ScriptLanguage language = ScriptLanguageFromString(lang);
					if (language == ScriptLanguage::kUnknownScriptLanguage)
					{
						LOG_ERROR("Invalid 'language' in script in 'script-list' in 'script-info.json'");
						throw std::runtime_error("Invalid file 'script-info.json'");
					}

					Ref<ScriptSource> script = AddSource(
						name,
						idIt->value.GetUint(),
						language,
						nullptr,
						0);
					if (script == nullptr)
					{
						LOG_ERROR("Add script '{0}'", name);
						throw std::runtime_error("Invalid file 'script-info.json'");
					}

					//Read source
					rapidjson::Value::MemberIterator sourceIt = scriptIt->FindMember("source");
					if (sourceIt != scriptIt->MemberEnd() && sourceIt->value.IsString())
					{
						std::string source(sourceIt->value.GetString(), sourceIt->value.GetStringLength());

						// Preventing user from loading files outside the default script directory
						if (source.find("..") != std::string::npos)
						{
							LOG_ERROR("Invalid 'source' in script in 'script-list' in 'script-info.json' because it contains a '..'");
							throw std::runtime_error("Invalid file 'script-info.json'");
						}

						std::string sourcePath = boost::filesystem::weakly_canonical(boost::filesystem::path("./scripts/" + source)).string();

						// Read source code
						std::ifstream sourceFile(sourcePath);
						if (sourceFile.is_open())
						{
							std::string sourceCode((std::istreambuf_iterator<char>(sourceFile)),
								std::istreambuf_iterator<char>());

							script->Update(reinterpret_cast<uint8_t*>(sourceCode.data()), sourceCode.size());

							sourceFile.close();
						}
					}
				}
			}
		}

		fclose(file);
	}
	void ScriptManager::Save()
	{
		boost::shared_lock_guard lock(mutex);

		LOG_INFO("Saving script information to 'script'");

		// Create json
		rapidjson::Document document = rapidjson::Document(rapidjson::kObjectType);

		rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

		// Create script list
		{
			rapidjson::Value scriptListJson = rapidjson::Value(rapidjson::kArrayType);

			// Save scripts
			for (std::pair<uint32_t, Ref<ScriptSource>> pair : scriptSourceList)
			{
				rapidjson::Value actionScriptJson = rapidjson::Value(rapidjson::kObjectType);

				pair.second->Save(actionScriptJson, allocator);

				scriptListJson.PushBack(actionScriptJson, allocator);
			}

			document.AddMember("script-list", scriptListJson, allocator);
		}

		//Save to file
		FILE* file = fopen("script-info.json", "w");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find 'script-info.json'");
			throw std::runtime_error("Open/find file 'script-info.json'");
		}

		char buffer[RAPIDJSON_BUFFER_SIZE_SMALL];
		rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));

		rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer = rapidjson::PrettyWriter<rapidjson::FileWriteStream>(stream);
		document.Accept(writer);

		fclose(file);
	}
}