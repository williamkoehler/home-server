#include "DynamicResources.hpp"
#include "../tools.hpp"

namespace server
{
	boost::weak_ptr<DynamicResources> instanceDynamicResources;

	DynamicResources::DynamicResources()
	{
	}
	DynamicResources::~DynamicResources()
	{
	}
	Ref<DynamicResources> DynamicResources::Create()
	{
		if (!instanceDynamicResources.expired())
			return Ref<DynamicResources>(instanceDynamicResources);

		Ref<DynamicResources> dynamicResources = boost::make_shared<DynamicResources>();
		instanceDynamicResources = dynamicResources;

		return dynamicResources;
	}
	Ref<DynamicResources> DynamicResources::GetInstance()
	{
		return Ref<DynamicResources>(instanceDynamicResources);
	}

	bool DynamicResources::HasResource(std::string name)
	{
		return resourceList.count(XXH32(name.c_str(), name.size(), 0x44595245)) > 0;
	}

	Ref<Resource> DynamicResources::AddResourceFromMemory(std::string prefix, std::string name, uint8_t* data, size_t size)
	{
		assert(size > 0);
		assert(data != nullptr);

		uint8_t* copy = new uint8_t[size];
		memcpy(copy, data, size);
		return AddResourceFromMemoryWithoutCopy(prefix, name, copy, size);
	}

	Ref<Resource> DynamicResources::AddResourceFromMemoryWithoutCopy(std::string prefix, std::string name, uint8_t* data, size_t size)
	{
		assert(size > 0);
		assert(data != nullptr);

		uint32_t id = XXH32(prefix.c_str(), prefix.size(), 0x44595245) ^ XXH32(name.c_str(), name.size(), 0x44595245);

		if (resourceList.count(id))
			return nullptr;

		Ref<Resource> resource = boost::make_shared<Resource>();

		resource->buffer = { data, size };

		boost::lock_guard lock(mutex);

		resourceList[id] = resource;

		LOG_INFO("Adding resource '{0}_{1}':{2} ({3})", prefix, name, id, PrettyBytes(size));

		return resource;
	}

	Ref<Resource> DynamicResources::UpdateResourceFromMemory(std::string prefix, std::string name, uint8_t* data, size_t size)
	{
		assert(size > 0);
		assert(data != nullptr);

		uint8_t* copy = new uint8_t[size];
		memcpy(copy, data, size);
		return UpdateResourceFromMemoryWithoutCopy(prefix, name, copy, size);
	}

	Ref<Resource> DynamicResources::UpdateResourceFromMemoryWithoutCopy(std::string prefix, std::string name, uint8_t* data, size_t size)
	{
		assert(size > 0);
		assert(data != nullptr);

		uint32_t id = XXH32(prefix.c_str(), prefix.size(), 0x44595245) ^ XXH32(name.c_str(), name.size(), 0x44595245);

		boost::lock_guard lock(mutex);

		Ref<Resource> resource;

		boost::unordered::unordered_map<uint32_t, Ref<Resource>>::iterator it = resourceList.find(id);
		if (it != resourceList.end())
		{
			resource = it->second;

			// Delete old data
			SAFE_DELETE_ARRAY(resource->buffer.data);

			LOG_INFO("Updating resource '{0}_{1}':{2} ({3})", prefix, name, id, PrettyBytes(size));
		}
		else
		{
			resource = boost::make_shared<Resource>();

			resourceList[id] = resource;

			LOG_INFO("Adding resource '{0}_{1}':{2} ({3})", prefix, name, id, PrettyBytes(size));
		}

		// Fill resource data
		resource->buffer = { data, size };

		return resource;
	}

	Ref<Resource> DynamicResources::GetFile(std::string prefix, std::string name)
	{
		boost::shared_lock_guard lock(mutex);

		uint32_t id = XXH32(prefix.c_str(), prefix.size(), 0x44595245) ^ XXH32(name.c_str(), name.size(), 0x44595245);

		boost::unordered::unordered_map<uint32_t, Ref<Resource>>::iterator it = resourceList.find(id);
		if (it == resourceList.end())
			return nullptr;

		return it->second;
	}

	void DynamicResources::RemoveResource(std::string prefix, std::string name)
	{
		uint32_t id = XXH32(prefix.c_str(), prefix.size(), 0x44595245) ^ XXH32(name.c_str(), name.size(), 0x44595245);

		boost::lock_guard lock(mutex);

		boost::unordered::unordered_map<uint32_t, Ref<Resource>>::iterator it = resourceList.find(id);
		if (it == resourceList.end())
			throw std::runtime_error("Resource ID does not exist");

		// Delete resource data
		SAFE_DELETE_ARRAY(it->second->buffer.data);

		// Remove resource from list
		resourceList.erase(it);
	}

	void DynamicResources::Load()
	{
		//boost::lock_guard lock(mutex);

		//LOG_INFO("Loading http resource information from file");

		//boost::filesystem::ifstream file("resources-info.json");

		//if (!file.is_open())
		//{
		//	LOG_ERROR("Failing to open/find 'resouces-info.json'");
		//	throw std::runtime_error("Open/find file 'resources-info.json'");
		//}

		////Parse
		//picojson::value json;
		//std::string err = picojson::parse(json, file);
		//file.close();
		//if (!err.empty())
		//{
		//	LOG_ERROR("Failing to read 'resouces-info.json' : {0}", err);
		//	throw std::runtime_error("Read file 'resouces-info.json'");
		//}

		////Load plugins
		//picojson::value& pluginDirectoryJson = json.get("plugin-directory");
		//if (pluginDirectoryJson.is<std::string>())
		//{
		//	pluginDirectory = pluginDirectoryJson.get<std::string>();

		//	pluginManager->Load(boost::filesystem::current_path().append("/" + pluginDirectory));
		//}
		//else
		//{
		//	LOG_WARNING("Missing 'plugin-directory' in 'resouces-info.json'. No plugins will be loaded.");
		//}
	}
	void DynamicResources::Save()
	{

	}
}