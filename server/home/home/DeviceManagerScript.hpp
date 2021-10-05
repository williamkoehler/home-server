#pragma once
#include "../common.hpp"
#include "../Script.hpp"
#include "Field.hpp"
#include "DeviceManager.hpp"
#include "../visual/Visual.hpp"

namespace server
{
	class JsonApi;
	class PluginManager;
	class DeviceManager;
}

namespace home
{
	class Plugin;

	typedef Ref<home::DeviceManagerScript>(CreateDeviceManagerScriptFunction)();

	class DeviceManagerScriptDescription
	{
	private:
		friend class server::JsonApi;
		friend class Plugin;
		friend class server::PluginManager;

		CreateDeviceManagerScriptFunction* createFunction = nullptr;
		std::string name;
		uint32_t scriptID = 0;

		//Visuals
		boost::container::vector<Ref<Visual>> propertyList;
	public:
		inline void AddProperty(const Ref<Visual>& visual)
		{
			propertyList.push_back(visual);
		}
	};

	class DeviceManagerScript : public Script, public boost::enable_shared_from_this<DeviceManagerScript>
	{
	protected:
		friend class server::DeviceManager;

		Ref<DeviceManager> deviceManager = nullptr;
	public:
		virtual void GetFields(home::WriteableFieldCollection& collection) = 0;
		virtual bool GetField(home::WriteableField& field) = 0;
		virtual bool SetField(home::ReadableField& field) = 0;
	};

#define DEVICEMANAGERSCRIPT_CLASS(_class, _name, _fields) \
	public: \
		_class() : home::DeviceManagerScript() {} \
		~_class() {} \
		static std::string _getScriptName() \
		{ \
			return _name; \
		} \
		virtual std::string GetScriptName() override \
		{ \
			return _name; \
		} \
		static uint32_t _getScriptID() \
		{ \
			static uint32_t scriptID = home::GenerateID(_name); \
			return scriptID; \
		} \
		virtual uint32_t GetScriptID() override \
		{ \
			return _getScriptID(); \
		} \
		static Ref<home::DeviceManagerScript> _createDeviceManagerScript() \
		{ \
			return boost::make_shared<_class>(); \
		} \
		_fields fields = {};
}
