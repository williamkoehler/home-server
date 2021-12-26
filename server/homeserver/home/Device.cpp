#include "Device.hpp"
#include "DeviceController.hpp"
#include "Room.hpp"
#include "../database/Database.hpp"

namespace server
{
	Device::Device(const std::string& name, identifier_t deviceID, Ref<home::DevicePlugin> plugin, Ref<DeviceController> controller, Ref<Room> room)
		: name(name), deviceID(deviceID), plugin(std::move(plugin)), controller(std::move(controller)), room(std::move(room))
	{
	}
	Device::~Device()
	{
	}
	Ref<Device> Device::Create(const std::string& name, identifier_t deviceID, Ref<home::DevicePlugin> plugin, Ref<DeviceController> controller, Ref<Room> room)
	{
		assert(plugin != nullptr);

		Ref<Device> device = boost::make_shared<Device>(name, deviceID, std::move(plugin), std::move(controller), std::move(room));

		return device;
	}

	Ref<home::DeviceController> Device::GetController()
	{
		boost::shared_lock_guard lock(mutex);
		return boost::reinterpret_pointer_cast<home::DeviceController>(controller);
	}

	std::string Device::GetName()
	{
		boost::lock_guard lock(mutex);
		return name;
	}
	bool Device::SetName(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDevicePropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		return false;
	}

	Ref<DeviceController> Device::GetController2()
	{
		boost::shared_lock_guard lock(mutex);
		return controller;
	}

	bool Device::SetController(Ref<DeviceController> v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDevicePropDeviceController(shared_from_this(), controller, v))
		{
			//if (controller != nullptr)
				//controller->RemoveDevice(shared_from_this());

			controller = v;

			//if (room != nullptr)
				//room->AddDevice(shared_from_this());
			return true;
		}
		return false;
	}

	Ref<Room> Device::GetRoom()
	{
		boost::shared_lock_guard lock(mutex);
		return room;
	}
	bool Device::SetRoom(Ref<Room> v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDevicePropRoom(shared_from_this(), room, v))
		{
			if (room != nullptr)
				room->RemoveDevice(shared_from_this());

			room = v;

			if (room != nullptr)
				room->AddDevice(shared_from_this());
			return true;
		}
		return false;
	}

	//! Interface: Attributes
	bool Device::AddAttribute(const std::string& id, const char* json)
	{
		boost::lock_guard lock(interfaceMutex);

		// Check existance
		if (!attributeList.contains(id))
		{
			// Parse attribute
			rapidjson::Document document;

			document.Parse(json);

			// Add attribute to list
			if (!document.HasParseError())
			{
				attributeList[id] = std::move(document);
				return true;
			}
		}

		return false;
	}
	bool Device::RemoveAttribute(const std::string& id)
	{
		boost::lock_guard lock(interfaceMutex);

		return attributeList.erase(id);
	}

	//! Interface: Properites
	Ref<home::Property> Device::AddProperty(const std::string& id, Ref<home::Property> property)
	{
		boost::lock_guard lock(interfaceMutex);

		// Check existance
		if (!propertyList.contains(id) && property != nullptr)
		{
			// Add property to list
			if (property != nullptr)
			{
				propertyList[id] = property;
				return property;
			}
		}

		return nullptr;
	}
	bool Device::RemoveProperty(const std::string& id)
	{
		boost::lock_guard lock(interfaceMutex);

		return propertyList.erase(id);
	}

	//! Interface: Events
	Ref<home::Event> Device::AddEvent(const std::string& id, home::EventCallback<home::Device>* callback)
	{
		boost::lock_guard lock(interfaceMutex);

		// Check existance
		if (!eventList.contains(id))
		{
			// Create timer instance
			Ref<Event> event = boost::make_shared<Event>(shared_from_this(), callback);

			// Add timer to list
			if (event != nullptr)
			{
				eventList[id] = event;
				return boost::reinterpret_pointer_cast<Event>(event);
			}
		}

		return nullptr;
	}
	bool Device::RemoveEvent(const std::string& id)
	{
		boost::lock_guard lock(interfaceMutex);

		return eventList.erase(id);
	}

	void Device::Invoke(const std::string& e)
	{
		boost::lock_guard lock(interfaceMutex);
		const robin_hood::unordered_node_map<std::string, Ref<Event>>::iterator it = eventList.find(e);

		if (it != eventList.end())
		{
			// Post job
			Ref<Event> event = it->second;
			Home::GetInstance()->GetService()->post([this, event]() -> void
			{
				if (event != nullptr)
					event->Invoke();
			});
		}
	}

	//! Interface: Timers
	Ref<home::Timer> Device::AddTimer(const std::string& id, home::TimerCallback<home::Device>* callback)
	{
		boost::lock_guard lock(interfaceMutex);

		// Check existance
		if (!timerList.contains(id))
		{
			// Create timer instance
			Ref<Timer> timer = boost::make_shared<Timer>(shared_from_this(), callback);

			// Add timer to list
			if (timer != nullptr)
			{
				timerList[id] = timer;
				return boost::reinterpret_pointer_cast<Timer>(timer);
			}
		}

		return nullptr;
	}
	bool Device::RemoveTimer(const std::string& id)
	{
		boost::lock_guard lock(interfaceMutex);

		const robin_hood::unordered_node_map<std::string, Ref<Timer>>::const_iterator it = timerList.find(id);
		if (it != timerList.end())
		{
			// Stop timer first
			it->second->Stop();

			timerList.erase(it);

			return true;
		}
		else
			return false;
	}

	void Device::Update()
	{ }

	void Device::Initialize()
	{
		try
		{
			if (!plugin->Initialize(boost::reinterpret_pointer_cast<home::Device>(shared_from_this())))
				LOG_ERROR("Failing to initialize plugin '{0}':{1}", name, deviceID);
		}
		catch (std::exception e)
		{
			LOG_ERROR("Failing to initialize plugin internal error '{0}':{1}", name, deviceID);
		}
	}

	void Device::TakeSnapshot()
	{
		boost::lock(mutex, interfaceMutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(interfaceMutex, boost::adopt_lock);

		rapidjson::Document::AllocatorType& allocator = snapshot.GetAllocator();

		snapshot.SetNull();
		allocator.Clear();

		snapshot.SetObject();

		snapshot.MemberReserve(propertyList.size(), allocator);
		for (robin_hood::pair<const std::string, Ref<home::Property>> pair : propertyList)
		{
			switch (pair.second->GetType())
			{
			case home::PropertyType::kBooleanType:
				snapshot.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), rapidjson::Value(pair.second->GetBoolean()), allocator);
				break;
			case home::PropertyType::kIntegerType:
				snapshot.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), rapidjson::Value(pair.second->GetInteger()), allocator);
				break;
			case home::PropertyType::kNumberType:
				snapshot.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), rapidjson::Value(pair.second->GetNumber()), allocator);
				break;
			case home::PropertyType::kStringType:
			{
				std::string value = pair.second->GetString();
				snapshot.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), rapidjson::Value(value.data(), value.size(), allocator), allocator);
				break;
			}
			case home::PropertyType::kEndpointType:
			{
				home::Endpoint endpoint = pair.second->GetEndpoint();

				rapidjson::Value endpointJson = rapidjson::Value(rapidjson::kObjectType);

				endpointJson.AddMember("class_", rapidjson::Value("endpoint"), allocator);
				endpointJson.AddMember("host", rapidjson::Value(endpoint.host.data(), endpoint.host.size(), allocator), allocator);
				endpointJson.AddMember("port", rapidjson::Value(endpoint.port), allocator);

				snapshot.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), endpointJson, allocator);
				break;
			}
			case home::PropertyType::kColorType:
			{
				home::Color color = pair.second->GetColor();

				rapidjson::Value endpointJson = rapidjson::Value(rapidjson::kObjectType);

				endpointJson.AddMember("class_", rapidjson::Value("color"), allocator);
				endpointJson.AddMember("r", rapidjson::Value(color.red), allocator);
				endpointJson.AddMember("g", rapidjson::Value(color.green), allocator);
				endpointJson.AddMember("b", rapidjson::Value(color.blue), allocator);

				snapshot.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), endpointJson, allocator);
				break;
			}
			default:
				snapshot.AddMember(rapidjson::Value(pair.first.data(), pair.first.size(), allocator), rapidjson::Value(rapidjson::kNullType), allocator);
				break;
			}
		}
	}

	void Device::Terminate()
	{
		try
		{
			if (!plugin->Initialize(boost::reinterpret_pointer_cast<home::Device>(shared_from_this())))
				LOG_ERROR("Failing to terminate plugin '{0}':{1}", name, deviceID);
		}
		catch (std::exception e)
		{
			LOG_ERROR("Failing to terminate plugin internal error '{0}':{1}", name, deviceID);
		}
	}
}