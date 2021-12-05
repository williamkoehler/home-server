#include "DeviceController.hpp"
#include <xxHash/xxhash.h>
#include "../plugin/PluginManager.hpp"
#include "Room.hpp"
#include "../database/Database.hpp"

namespace server
{
	DeviceController::DeviceController(const std::string& name, identifier_t controllerID, Ref<home::DeviceControllerPlugin> plugin, Ref<Room> room)
		: name(name), controllerID(controllerID), plugin(std::move(plugin)), room(std::move(room))
	{
	}
	DeviceController::~DeviceController()
	{
		try
		{
			if (!plugin->Terminate(boost::reinterpret_pointer_cast<home::DeviceController>(shared_from_this())))
				LOG_ERROR("Failing to terminate plugin {0}:{1}", name, controllerID);
		}
		catch (std::exception)
		{
			LOG_ERROR("Failing to initialize plugin internal error {0}:{1}", name, controllerID);
		}
	}
	Ref<DeviceController> DeviceController::Create(const std::string& name, identifier_t controllerID, Ref<home::DeviceControllerPlugin> plugin, Ref<Room> room)
	{
		assert(plugin != nullptr);

		Ref<DeviceController> controller = boost::make_shared<DeviceController>(name, controllerID, std::move(plugin), std::move(room));

		if (controller != nullptr)
		{
			try
			{
				if (!controller->plugin->Initialize(boost::reinterpret_pointer_cast<home::DeviceController>(controller)))
					LOG_ERROR("Failing to initialize plugin {0}:{1}", name, controllerID);
			}
			catch (std::exception)
			{
				LOG_ERROR("Failing to initialize plugin internal error {0}:{1}", name, controllerID);
			}
		}

		return controller;
	}

	//! Internal: Name
	std::string DeviceController::GetName()
	{
		boost::lock_guard lock(mutex);
		return name;
	}
	bool DeviceController::SetName(const std::string& v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDeviceControllerPropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		return false;
	}

	//! Internal: Room
	Ref<Room> DeviceController::GetRoom()
	{
		boost::shared_lock_guard lock(mutex);
		return room;
	}
	bool DeviceController::SetRoom(Ref<Room> v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateDeviceControllerPropRoom(shared_from_this(), room, v))
		{
			if (room != nullptr)
				room->RemoveDeviceController(shared_from_this());

			room = v;

			if (room != nullptr)
				room->AddDeviceController(shared_from_this());
			return true;
		}
		return false;
	}

	//! Interface: Attributes
	bool DeviceController::AddAttribute(const std::string& id, const char* json)
	{
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
	bool DeviceController::RemoveAttribute(const std::string& id)
	{
		return attributeList.erase(id);
	}

	//! Interface: Properites
	Ref<home::Property> DeviceController::AddProperty(const std::string& id, Ref<home::Property> property)
	{
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
	bool DeviceController::RemoveProperty(const std::string& id)
	{
		return propertyList.erase(id);
	}

	//! Interface: Events
	Ref<home::Event> DeviceController::AddEvent(const std::string& id, home::EventCallback<home::DeviceController>* callback)
	{
		// Check existance
		if (!eventList.contains(id))
		{
			// Create timer instance
			Ref<Event> event = boost::make_shared<Event>(shared_from_this(), callback);

			// Add timer to list
			if (event != nullptr)
			{
				eventList[id] = event;
				return boost::reinterpret_pointer_cast<home::Event>(event);
			}
		}

		return nullptr;
	}
	bool DeviceController::RemoveEvent(const std::string& id)
	{
		return eventList.erase(id);
	}

	void DeviceController::Invoke(const std::string& e)
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
	Ref<home::Timer> DeviceController::AddTimer(const std::string& id, home::TimerCallback<home::DeviceController>* callback)
	{
		// Check existance
		if (!timerList.contains(id))
		{
			// Create timer instance
			Ref<Timer> timer = boost::make_shared<Timer>(shared_from_this(), callback);

			// Add timer to list
			if (timer != nullptr)
			{
				timerList[id] = timer;
				return boost::reinterpret_pointer_cast<home::Timer>(timer);
			}
		}

		return nullptr;
	}
	bool DeviceController::RemoveTimer(const std::string& id)
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

	void DeviceController::TakeSnapshot()
	{
		boost::lock_guard lock(mutex);

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
}