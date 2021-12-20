#include "JsonApi.hpp"
#include "../../tools.hpp"

#include "../../home/Home.hpp"
#include "../../home/Room.hpp"
#include "../../home/DeviceController.hpp"
#include "../../home/Device.hpp"
#include "../../home/Action.hpp"

namespace server
{
	// Home
	void JsonApi::BuildJsonHome(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator, size_t timestamp)
	{
		assert(output.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		boost::shared_lock_guard lock(home->mutex);

		output.AddMember("timestamp", rapidjson::Value(home->timestamp), allocator);

		// Only send tree if it has changed
		if (home->timestamp > timestamp)
		{
			// Devices
			rapidjson::Value deviceListJson = rapidjson::Value(rapidjson::kArrayType);

			// Reserve memory
			deviceListJson.Reserve(home->deviceList.size(), allocator);

			for (robin_hood::pair<const identifier_t, Ref<Device>> item : home->deviceList)
			{
				rapidjson::Value deviceJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonDevice(item.second, deviceJson, allocator);

				deviceListJson.PushBack(deviceJson, allocator);
			}

			output.AddMember("devices", deviceListJson, allocator);

			// DeviceControllers
			rapidjson::Value deviceControllerListJson = rapidjson::Value(rapidjson::kArrayType);

			// Reserve memory
			deviceControllerListJson.Reserve(home->deviceControllerList.size(), allocator);

			for (robin_hood::pair<const identifier_t, Ref<DeviceController>> item : home->deviceControllerList)
			{
				rapidjson::Value deviceControllerJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonDeviceController(item.second, deviceControllerJson, allocator);

				deviceControllerListJson.PushBack(deviceControllerJson, allocator);
			}

			output.AddMember("devicecontrollers", deviceControllerListJson, allocator);

			// Rooms
			rapidjson::Value roomListJson = rapidjson::Value(rapidjson::kArrayType);

			// Reserve memory
			roomListJson.Reserve(home->roomList.size(), allocator);

			for (robin_hood::pair<const identifier_t, Ref<Room>> item : home->roomList)
			{
				rapidjson::Value roomJson = rapidjson::Value(rapidjson::kObjectType);

				BuildJsonRoom(item.second, roomJson, allocator);

				roomListJson.PushBack(roomJson, allocator);
			}

			output.AddMember("rooms", roomListJson, allocator);
		}
	}
	void JsonApi::DecodeJsonHome(rapidjson::Value& input)
	{
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode devices if it exists
		rapidjson::Value::MemberIterator deviceListIt = input.FindMember("devices");
		if (deviceListIt != input.MemberEnd() && deviceListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator deviceIt = deviceListIt->value.Begin(); deviceIt != deviceListIt->value.End(); deviceIt++)
			{
				if (deviceIt->IsObject())
				{
					rapidjson::Value::MemberIterator deviceIDIt = input.FindMember("id");
					if (deviceIDIt == input.MemberEnd() || !deviceIDIt->value.IsUint())
						return;

					Ref<Device> device = home->GetDevice(deviceIDIt->value.GetUint64());
					if (device != nullptr)
						DecodeJsonDevice(device, *deviceIt);
				}
			}
		}

		// Decode devicecontrollers if it exists
		rapidjson::Value::MemberIterator deviceControllerListIt = input.FindMember("devicecontrollers");
		if (deviceControllerListIt != input.MemberEnd() && deviceControllerListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator deviceControllerIt = deviceControllerListIt->value.Begin(); deviceControllerIt != deviceControllerListIt->value.End(); deviceControllerIt++)
			{
				if (deviceControllerIt->IsObject())
				{
					rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("id");
					if (controllerIDIt == input.MemberEnd() || !controllerIDIt->value.IsUint())
						return;

					Ref<DeviceController> controller = home->GetDeviceController(controllerIDIt->value.GetUint64());
					if (controller != nullptr)
						DecodeJsonDeviceController(controller, *deviceControllerIt);
				}
			}
		}

		// Decode rooms if it exists
		rapidjson::Value::MemberIterator roomListIt = input.FindMember("rooms");
		if (roomListIt != input.MemberEnd() && roomListIt->value.IsArray())
		{
			for (rapidjson::Value::ValueIterator roomIt = roomListIt->value.Begin(); roomIt != roomListIt->value.End(); roomIt++)
			{
				if (roomIt->IsObject())
				{
					rapidjson::Value::MemberIterator roomIDIt = input.FindMember("id");
					if (roomIDIt == input.MemberEnd() || !roomIDIt->value.IsUint())
						return;

					Ref<Room> room = home->GetRoom(roomIDIt->value.GetUint64());
					if (room != nullptr)
						DecodeJsonRoom(room, *roomIt);
				}
			}
		}
	}

	void JsonApi::BuildJsonRoom(Ref<Room> room, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(room != nullptr);
		assert(output.IsObject());

		boost::shared_lock_guard lock(room->mutex);

		output.AddMember("name", rapidjson::Value(room->name.c_str(), room->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(room->roomID), allocator);
		output.AddMember("type", rapidjson::Value(room->type.c_str(), room->type.size()), allocator);

		// Build devices
		boost::container::set<identifier_t>& deviceList = room->deviceList;

		// Reserve memory
		rapidjson::Value devicesJson = rapidjson::Value(rapidjson::kArrayType);
		devicesJson.Reserve(deviceList.size(), allocator);

		for (identifier_t& deviceID : deviceList)
			devicesJson.PushBack(rapidjson::Value(deviceID), allocator);

		output.AddMember("devices", devicesJson, allocator);

		// Build devicecontrollers
		boost::container::set<identifier_t>& deviceControllerList = room->deviceControllerList;

		// Reserve memory
		rapidjson::Value devicecontrollersJson = rapidjson::Value(rapidjson::kArrayType);
		devicecontrollersJson.Reserve(deviceList.size(), allocator);

		for (identifier_t& controllerID : deviceControllerList)
			devicecontrollersJson.PushBack(rapidjson::Value(controllerID), allocator);

		output.AddMember("devicecontrollers", devicecontrollersJson, allocator);
	}
	void JsonApi::DecodeJsonRoom(Ref<Room> room, rapidjson::Value& input)
	{
		assert(room != nullptr);
		assert(input.IsObject());

		// Decode properties
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		if (nameIt != input.MemberEnd() && nameIt->value.IsString())
			room->SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

		rapidjson::Value::MemberIterator typeIt = input.FindMember("type");
		if (typeIt != input.MemberEnd() && typeIt->value.IsUint())
			room->SetType(std::string(typeIt->value.GetString(), typeIt->value.GetStringLength()));
	}

	void JsonApi::BuildJsonDeviceController(Ref<DeviceController> controller, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(controller != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(controller->mutex);

		// Build properties
		output.AddMember("name", rapidjson::Value(controller->name.c_str(), controller->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(controller->controllerID), allocator);
		output.AddMember("pluginid", rapidjson::Value(controller->GetPluginID()), allocator);
		output.AddMember("roomid",
			controller->room != nullptr ?
			rapidjson::Value(controller->room->GetRoomID()) : rapidjson::Value(rapidjson::kNullType), allocator);
	}
	void JsonApi::DecodeJsonDeviceController(Ref<DeviceController> controller, rapidjson::Value& input)
	{
		assert(controller != nullptr);
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode properties
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		if (nameIt != input.MemberEnd() && nameIt->value.IsString())
			controller->SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (roomIDIt != input.MemberEnd() && roomIDIt->value.IsUint64())
			controller->SetRoom(home->GetRoom(roomIDIt->value.GetUint64()));
	}
	void JsonApi::BuildJsonDeviceControllerState(Ref<DeviceController> controller, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(controller != nullptr);
		assert(output.IsObject());

		// Lock mutex
		boost::shared_lock_guard lock(controller->mutex);

		// Build properties
		output.AddMember("id", rapidjson::Value(controller->controllerID), allocator);
		output.AddMember("state", rapidjson::Value(controller->snapshot, allocator), allocator);
	}
	void JsonApi::DecodeJsonDeviceControllerState(Ref<DeviceController> controller, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(controller != nullptr);
		assert(input.IsObject());

		// Decode device properties if they exist
		{
			rapidjson::Value::MemberIterator stateIt = input.FindMember("state");
			if (stateIt != input.MemberEnd() && stateIt->value.IsObject())
			{

				boost::lock_guard lock(controller->interfaceMutex);

				// Parse every property
				for (rapidjson::Value::MemberIterator propertyIt = stateIt->value.MemberBegin(); propertyIt != stateIt->value.MemberEnd(); propertyIt++)
				{
					const robin_hood::unordered_node_map<std::string, Ref<home::Property>>::const_iterator it = controller->propertyList.find(std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength()));
					if (it != controller->propertyList.end())
					{
						switch (propertyIt->value.GetType())
						{
						case rapidjson::kFalseType:
							it->second->SetBoolean(false);
							break;
						case rapidjson::kTrueType:
							it->second->SetBoolean(true);
							break;
						case rapidjson::kNumberType:
							if (it->second->IsInteger())
								it->second->SetInteger(propertyIt->value.GetInt64());
							else
								it->second->SetNumber(propertyIt->value.GetDouble());
							break;
						case rapidjson::kStringType:
							it->second->SetString(std::string(propertyIt->value.GetString(), propertyIt->value.GetStringLength()));
							break;
						case rapidjson::kObjectType:
						{
							rapidjson::Value::MemberIterator classIt = propertyIt->value.FindMember("class_");
							if (classIt != propertyIt->value.MemberEnd() && classIt->value.IsString())
							{
								switch (crc32(classIt->value.GetString(), classIt->value.GetStringLength()))
								{
								case CRC32("endpoint"):
								{
									// Parse endpoint
									rapidjson::Value::MemberIterator hostIt = propertyIt->value.FindMember("host");
									rapidjson::Value::MemberIterator portIt = propertyIt->value.FindMember("port");

									if (hostIt != propertyIt->value.MemberEnd() && hostIt->value.IsString() &&
										portIt != propertyIt->value.MemberEnd() && portIt->value.IsUint())
									{
										// Set endpoint
										it->second->SetEndpoint(home::Endpoint{ std::string(hostIt->value.GetString(), hostIt->value.GetStringLength()), (uint16_t)portIt->value.GetUint() });
									}
									break;
								}
								case CRC32("color"):
								{
									rapidjson::Value::MemberIterator redIt = propertyIt->value.FindMember("r");
									rapidjson::Value::MemberIterator greenIt = propertyIt->value.FindMember("g");
									rapidjson::Value::MemberIterator blueIt = propertyIt->value.FindMember("b");

									if (redIt != propertyIt->value.MemberEnd() && redIt->value.IsUint() &&
										greenIt != propertyIt->value.MemberEnd() && greenIt->value.IsUint() &&
										blueIt != propertyIt->value.MemberEnd() && blueIt->value.IsUint())
									{
										// Set color
										it->second->SetColor(home::Color{ (uint8_t)redIt->value.GetUint(), (uint8_t)greenIt->value.GetUint(), (uint8_t)blueIt->value.GetUint() });
									}
									break;
								}
								}
							}
							break;
						}
						default:
							break;
						}
					}
				}
			}

			controller->TakeSnapshot();

			boost::lock_guard lock(controller->mutex);
			output.AddMember("state", rapidjson::Value(controller->snapshot, allocator), allocator);
		}
	}

	void JsonApi::BuildJsonDevice(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(device->mutex);

		// Build properties
		output.AddMember("name", rapidjson::Value(device->name.c_str(), device->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(device->deviceID), allocator);
		output.AddMember("pluginid", rapidjson::Value(device->GetPluginID()), allocator);
		output.AddMember("controllerid",
			device->controller != nullptr ?
			rapidjson::Value(device->controller->GetDeviceControllerID()) : rapidjson::Value(rapidjson::kNullType), allocator);
		output.AddMember("roomid",
			device->room != nullptr ?
			rapidjson::Value(device->room->GetRoomID()) : rapidjson::Value(rapidjson::kNullType), allocator);
	}
	void JsonApi::DecodeJsonDevice(Ref<Device> device, rapidjson::Value& input)
	{
		assert(device != nullptr);
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode properties
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		if (nameIt != input.MemberEnd() && nameIt->value.IsString())
			device->SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

		rapidjson::Value::MemberIterator controllerIDIt = input.FindMember("controllerid");
		if (controllerIDIt != input.MemberEnd() && controllerIDIt->value.IsUint64())
			device->SetController(home->GetDeviceController(controllerIDIt->value.GetInt64()));

		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (roomIDIt != input.MemberEnd() && roomIDIt->value.IsUint64())
			device->SetRoom(home->GetRoom(roomIDIt->value.GetUint64()));
	}
	void JsonApi::BuildJsonDeviceState(Ref<Device> device, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(device->mutex);

		// Build properties
		output.AddMember("id", rapidjson::Value(device->deviceID), allocator);
		output.AddMember("state", rapidjson::Value(device->snapshot, allocator), allocator);
	}
	void JsonApi::DecodeJsonDeviceState(Ref<Device> device, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(device != nullptr);
		assert(input.IsObject());

		// Decode device properties if they exist
		{
			rapidjson::Value::MemberIterator stateIt = input.FindMember("state");
			if (stateIt != input.MemberEnd() && stateIt->value.IsObject())
			{
				boost::lock_guard lock(device->interfaceMutex);

				// Parse every property
				for (rapidjson::Value::MemberIterator propertyIt = stateIt->value.MemberBegin(); propertyIt != stateIt->value.MemberEnd(); propertyIt++)
				{
					const robin_hood::unordered_node_map<std::string, Ref<home::Property>>::const_iterator it = device->propertyList.find(std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength()));
					if (it != device->propertyList.end())
					{
						switch (propertyIt->value.GetType())
						{
						case rapidjson::kFalseType:
							it->second->SetBoolean(false);
							break;
						case rapidjson::kTrueType:
							it->second->SetBoolean(true);
							break;
						case rapidjson::kNumberType:
							if (it->second->IsInteger())
								it->second->SetInteger(propertyIt->value.GetInt64());
							else
								it->second->SetNumber(propertyIt->value.GetDouble());
							break;
						case rapidjson::kStringType:
							it->second->SetString(std::string(propertyIt->value.GetString(), propertyIt->value.GetStringLength()));
							break;
						case rapidjson::kObjectType:
						{
							rapidjson::Value::MemberIterator classIt = propertyIt->value.FindMember("class_");
							if (classIt != propertyIt->value.MemberEnd() && classIt->value.IsString())
							{
								switch (crc32(classIt->value.GetString(), classIt->value.GetStringLength()))
								{
								case CRC32("endpoint"):
								{
									// Parse endpoint
									rapidjson::Value::MemberIterator hostIt = propertyIt->value.FindMember("host");
									rapidjson::Value::MemberIterator portIt = propertyIt->value.FindMember("port");

									if (hostIt != propertyIt->value.MemberEnd() && hostIt->value.IsString() &&
										portIt != propertyIt->value.MemberEnd() && portIt->value.IsUint())
									{
										// Set endpoint
										it->second->SetEndpoint(home::Endpoint{ std::string(hostIt->value.GetString(), hostIt->value.GetStringLength()), (uint16_t)portIt->value.GetUint() });
									}
									break;
								}
								case CRC32("color"):
								{
									rapidjson::Value::MemberIterator redIt = propertyIt->value.FindMember("r");
									rapidjson::Value::MemberIterator greenIt = propertyIt->value.FindMember("g");
									rapidjson::Value::MemberIterator blueIt = propertyIt->value.FindMember("b");

									if (redIt != propertyIt->value.MemberEnd() && redIt->value.IsUint() &&
										greenIt != propertyIt->value.MemberEnd() && greenIt->value.IsUint() &&
										blueIt != propertyIt->value.MemberEnd() && blueIt->value.IsUint())
									{
										// Set color
										it->second->SetColor(home::Color{ (uint8_t)redIt->value.GetUint(), (uint8_t)greenIt->value.GetUint(), (uint8_t)blueIt->value.GetUint() });
									}
									break;
								}
								}
							}
							break;
						}
						default:
							break;
						}
					}
				}
			}

			device->TakeSnapshot();

			boost::lock_guard lock(device->mutex);
			output.AddMember("state", rapidjson::Value(device->snapshot, allocator), allocator);
		}
	}

	void JsonApi::BuildJsonAction(Ref<Action> action, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(action != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(action->mutex);

		// Build properties
		output.AddMember("name", rapidjson::Value(action->name.c_str(), action->name.size()), allocator);
		output.AddMember("id", rapidjson::Value(action->actionID), allocator);
		output.AddMember("sourceid", rapidjson::Value(action->GetScriptSourceID()), allocator);
		output.AddMember("roomid",
			action->room != nullptr ?
			rapidjson::Value(action->room->GetRoomID()) : rapidjson::Value(rapidjson::kNullType), allocator);
	}
	void JsonApi::DecodeJsonAction(Ref<Action> action, rapidjson::Value& input)
	{
		assert(action != nullptr);
		assert(input.IsObject());

		Ref<Home> home = Home::GetInstance();
		assert(home != nullptr);

		// Decode properties
		rapidjson::Value::MemberIterator nameIt = input.FindMember("name");
		if (nameIt != input.MemberEnd() && nameIt->value.IsString())
			action->SetName(std::string(nameIt->value.GetString(), nameIt->value.GetStringLength()));

		rapidjson::Value::MemberIterator roomIDIt = input.FindMember("roomid");
		if (roomIDIt != input.MemberEnd() && roomIDIt->value.IsUint64())
			action->SetRoom(home->GetRoom(roomIDIt->value.GetUint64()));
	}
	void JsonApi::BuildJsonActionState(Ref<Action> action, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(action != nullptr);
		assert(output.IsObject());

		// Lock
		boost::shared_lock_guard lock(action->mutex);

		// Build properties
		output.AddMember("id", rapidjson::Value(action->actionID), allocator);
		output.AddMember("state", rapidjson::Value(action->snapshot, allocator), allocator);
	}
	void JsonApi::DecodeJsonActionState(Ref<Action> action, rapidjson::Value& input, rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
	{
		assert(action != nullptr);
		assert(input.IsObject());

		// Decode device properties if they exist
		{
			rapidjson::Value::MemberIterator stateIt = input.FindMember("state");
			if (stateIt != input.MemberEnd() && stateIt->value.IsObject())
			{
				boost::lock_guard lock(action->interfaceMutex);

				// Parse every property
				for (rapidjson::Value::MemberIterator propertyIt = stateIt->value.MemberBegin(); propertyIt != stateIt->value.MemberEnd(); propertyIt++)
				{
					const robin_hood::unordered_node_map<std::string, Ref<home::Property>>::const_iterator it = action->propertyList.find(std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength()));
					if (it != action->propertyList.end())
					{
						switch (propertyIt->value.GetType())
						{
						case rapidjson::kFalseType:
							it->second->SetBoolean(false);
							break;
						case rapidjson::kTrueType:
							it->second->SetBoolean(true);
							break;
						case rapidjson::kNumberType:
							if (it->second->IsInteger())
								it->second->SetInteger(propertyIt->value.GetInt64());
							else
								it->second->SetNumber(propertyIt->value.GetDouble());
							break;
						case rapidjson::kStringType:
							it->second->SetString(std::string(propertyIt->value.GetString(), propertyIt->value.GetStringLength()));
							break;
						case rapidjson::kObjectType:
						{
							rapidjson::Value::MemberIterator classIt = propertyIt->value.FindMember("class_");
							if (classIt != propertyIt->value.MemberEnd() && classIt->value.IsString())
							{
								switch (crc32(classIt->value.GetString(), classIt->value.GetStringLength()))
								{
								case CRC32("endpoint"):
								{
									// Parse endpoint
									rapidjson::Value::MemberIterator hostIt = propertyIt->value.FindMember("host");
									rapidjson::Value::MemberIterator portIt = propertyIt->value.FindMember("port");

									if (hostIt != propertyIt->value.MemberEnd() && hostIt->value.IsString() &&
										portIt != propertyIt->value.MemberEnd() && portIt->value.IsUint())
									{
										// Set endpoint
										it->second->SetEndpoint(home::Endpoint{ std::string(hostIt->value.GetString(), hostIt->value.GetStringLength()), (uint16_t)portIt->value.GetUint() });
									}
									break;
								}
								case CRC32("color"):
								{
									rapidjson::Value::MemberIterator redIt = propertyIt->value.FindMember("r");
									rapidjson::Value::MemberIterator greenIt = propertyIt->value.FindMember("g");
									rapidjson::Value::MemberIterator blueIt = propertyIt->value.FindMember("b");

									if (redIt != propertyIt->value.MemberEnd() && redIt->value.IsUint() &&
										greenIt != propertyIt->value.MemberEnd() && greenIt->value.IsUint() &&
										blueIt != propertyIt->value.MemberEnd() && blueIt->value.IsUint())
									{
										// Set color
										it->second->SetColor(home::Color{ (uint8_t)redIt->value.GetUint(), (uint8_t)greenIt->value.GetUint(), (uint8_t)blueIt->value.GetUint() });
									}
									break;
								}
								}
							}
							break;
						}
						default:
							break;
						}
					}
				}
			}

			action->TakeSnapshot();

			boost::lock_guard lock(action->mutex);
			output.AddMember("state", rapidjson::Value(action->snapshot, allocator), allocator);
		}
	}
}