#include "DraftSource.h"

namespace scripting
{
	DraftSource::DraftSource(std::string name, uint32_t sourceID, DraftLanguages language)
		: name(std::move(name)), sourceID(sourceID), language(language), data(nullptr), length(0)
	{ }
	DraftSource::~DraftSource()
	{
		free(data);
	}
	Ref<DraftSource> DraftSource::Create(std::string name, uint32_t sourceID, DraftLanguages language)
	{
		Ref<DraftSource> source = boost::make_shared<DraftSource>(std::move(name), sourceID, language);

		// Initialize memory
		source->length = 1024;
		source->data = static_cast<uint8_t*>(malloc(source->length));
		if (source->data == nullptr)
		{
			LOG_MEMORY_ALLOCATION("Create script source");
			return nullptr;
		}

		return source;
	}

	void DraftSource::Update(uint8_t* d, size_t l)
	{
		boost::lock_guard lock(mutex);

		length = l;
		data = static_cast<uint8_t*>(realloc(data, length));
		if (data == nullptr)
		{
			LOG_MEMORY_ALLOCATION("Update script source");
			return;
		}

		memcpy(data, d, length);
	}

	void DraftSource::Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator)
	{
		assert(json.IsObject());

		boost::lock_guard lock(mutex);

		json.AddMember("name", rapidjson::Value(name.c_str(), name.size(), allocator), allocator);
		json.AddMember("id", sourceID, allocator);

		const DraftLanguages language = GetLanguage();
		const std::string lang = scripting::DraftLanguageToString(language);
		json.AddMember("language", rapidjson::Value(lang.c_str(), lang.size(), allocator), allocator);

		// Write source code to file
		const std::string source = "script_" + std::to_string(sourceID) + DraftExtFromDraftLanguage(language);
		json.AddMember("source", rapidjson::Value(source.c_str(), source.size(), allocator), allocator);

		const std::string sourcePath = boost::filesystem::weakly_canonical(boost::filesystem::path("./scripts/" + source)).string();

		FILE* file = fopen("./scripts/", "w");
		if (file == nullptr)
		{
			LOG_ERROR("Open/find '{0}'", sourcePath);
			return;
		}

		// Write to file
		fwrite(data, sizeof(uint8_t), length, file);

		// Close file
		fclose(file);
	}
}
