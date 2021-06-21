#pragma once
#include "../common.h"
#include "../helper/Types.h"
#include "../Script.h"
#include "Field.h"
#include "Device.h"
#include "../visual/Visual.h"

namespace server
{
	class JsonApi;
	class PluginManager;
	class Device;
}

namespace home
{
	class Plugin;
	class DeviceManager;
	class DeviceManagerScript;

	typedef Ref<DeviceScript>(CreateDeviceScriptFunction)();

	enum DeviceTypes : uint32_t
	{
		kUnknownDeviceType = 0,
		kLightDeviceType,
		kColoredLightDeviceType,
		kThermometerDeviceType,
	};

	class DeviceScriptDescription
	{
	private:
		friend class server::JsonApi;
		friend class Plugin;
		friend class server::PluginManager;

		CreateDeviceScriptFunction* createFunction = nullptr;
		std::string name;
		uint32_t type = 0;
		uint32_t scriptID = 0;

		// Visuals
		boost::container::vector<Ref<Visual>> propertyList;
		boost::container::vector<Ref<Visual>> parameterList;
	public:
		inline void AddProperty(const Ref<Visual>& visual)
		{
			propertyList.push_back(visual);
		}
		inline void AddParameter(const Ref<Visual>& visual)
		{
			parameterList.push_back(visual);
		}
	};

	class HOME_EXPORT DeviceScript : public Script, public boost::enable_shared_from_this<DeviceScript>
	{
	protected:
		friend class server::Device;

		Ref<Device> device = nullptr;
	public:
		virtual bool HasError() = 0;
		virtual std::string GetError() = 0;

		virtual void GetFields(home::WriteableFieldCollection& collection) = 0;
		virtual bool GetField(WriteableField& field) = 0;
		virtual bool SetField(ReadableField& field) = 0;
	};

#define DEVICESCRIPT_CLASS(_class, _name, _type, _fields) \
	public: \
		_class() : home::DeviceScript() {} \
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
		static uint32_t _getType() \
		{ \
			return (uint32_t)(_type); \
		} \
		static Ref<home::DeviceScript> _createDeviceScript() \
		{ \
			return boost::make_shared<_class>(); \
		} \
		_fields fields = {};
}