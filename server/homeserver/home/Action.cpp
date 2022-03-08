#include "Action.hpp"
#include "Room.hpp"
#include "../database/Database.hpp"

namespace server
{
	Action::Action(const std::string &name, identifier_t actionID, Ref<Script> script, Ref<Room> room)
		: name(name), actionID(actionID), script(std::move(script)), room(std::move(room))
	{
	}
	Action::~Action()
	{
	}
	Ref<Action> Action::Create(const std::string &name, identifier_t actionID, Ref<Script> script, Ref<Room> room)
	{
		assert(script != nullptr);

		Ref<Action> action = boost::make_shared<Action>(name, actionID, std::move(script), std::move(room));

		if (action != nullptr)
		{
			action->Update();
		}

		return action;
	}

	std::string Action::GetName()
	{
		boost::lock_guard lock(mutex);
		return name;
	}
	bool Action::SetName(const std::string &v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateActionPropName(shared_from_this(), name, v))
		{
			name = v;
			return true;
		}
		else
			return false;
	}

	Ref<Room> Action::GetRoom()
	{
		boost::shared_lock_guard lock(mutex);
		return room;
	}
	bool Action::SetRoom(Ref<Room> v)
	{
		boost::lock_guard lock(mutex);
		if (Database::GetInstance()->UpdateActionPropRoom(shared_from_this(), room, v))
		{
			if (room != nullptr)
				room->RemoveAction(shared_from_this());

			room = v;

			if (room != nullptr)
				room->AddAction(shared_from_this());
			return true;
		}
		return false;
	}

	//! Interface: Attributes
	bool Action::AddAttribute(const std::string &id, const char *json)
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
	bool Action::RemoveAttribute(const std::string &id)
	{
		boost::lock_guard lock(interfaceMutex);

		return attributeList.erase(id);
	}
	void Action::ClearAttributes()
	{
		boost::lock_guard lock(interfaceMutex);

		attributeList.clear();
	}

	//! Interface: Properites
	Ref<home::Property> Action::AddProperty(const std::string &id, home::PropertyType type)
	{
		boost::lock_guard lock(interfaceMutex);

		// Check existance
		if (!propertyList.contains(id))
		{

			// Create property instance
			Ref<home::Property> property;
			switch (type)
			{
			case home::PropertyType::kBooleanType:
				property = home::BooleanProperty::Create();
				break;
			case home::PropertyType::kIntegerType:
				property = home::IntegerProperty::Create();
				break;
			case home::PropertyType::kNumberType:
				property = home::NumberProperty::Create();
				break;
			case home::PropertyType::kStringType:
				property = home::StringProperty::Create();
				break;
			case home::PropertyType::kEndpointType:
				property = home::EndpointProperty::Create();
				break;
			case home::PropertyType::kColorType:
				property = home::ColorProperty::Create();
				break;
			default:
				property = nullptr;
				break;
			}

			// Add property to list
			if (property != nullptr)
			{
				propertyList[id] = property;
				return property;
			}
		}

		return nullptr;
	}
	bool Action::RemoveProperty(const std::string &id)
	{
		boost::lock_guard lock(interfaceMutex);

		return propertyList.erase(id);
	}
	void Action::ClearProperties()
	{
		boost::lock_guard lock(interfaceMutex);

		propertyList.clear();
	}

	//! Interface: Events
	Ref<home::Event> Action::AddEvent(const std::string &id, const std::string &callback)
	{
		boost::lock_guard lock(interfaceMutex);

		// Check existance
		if (!eventList.contains(id))
		{
			// Create event instance
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
	bool Action::RemoveEvent(const std::string &id)
	{
		boost::lock_guard lock(interfaceMutex);

		return eventList.erase(id);
	}
	void Action::ClearEvents()
	{
		boost::lock_guard lock(interfaceMutex);

		eventList.clear();
	}

	void Action::Invoke(const std::string &e)
	{
		boost::lock_guard lock(interfaceMutex);
		const robin_hood::unordered_node_map<std::string, Ref<Event>>::iterator it = eventList.find(e);

		if (it != eventList.end())
		{
			// Post job
			Ref<Event> event = it->second;
			boost::asio::post(
				*Home::GetInstance()->GetService(),
				boost::bind(&Event::Invoke, event));
		}
	}

	//! Interface: Timers
	Ref<home::Timer> Action::AddTimer(const std::string &id, const std::string &callback)
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
	bool Action::RemoveTimer(const std::string &id)
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
	void Action::ClearTimers()
	{
		boost::lock_guard lock(interfaceMutex);

		timerList.clear();
	}

	void Action::Update()
	{
		boost::asio::post(
			*Home::GetInstance()->GetService(),
			boost::bind(&server::Script::Prepare, script, boost::reinterpret_pointer_cast<Scriptable>(shared_from_this())));
	}

	void Action::TakeSnapshot()
	{
		boost::lock(mutex, interfaceMutex);
		boost::lock_guard lock(mutex, boost::adopt_lock);
		boost::lock_guard lock2(interfaceMutex, boost::adopt_lock);

		rapidjson::Document::AllocatorType &allocator = snapshot.GetAllocator();

		snapshot.SetNull();
		allocator.Clear();

		snapshot.SetObject();

		snapshot.MemberReserve(propertyList.size(), allocator);
		for (robin_hood::pair<const std::string, Ref<home::Property>> pair : propertyList)
		{
			// Add property
			snapshot.AddMember(
				rapidjson::Value(pair.first.data(), pair.first.size(), allocator),
				pair.second->ToJson(allocator),
				allocator);
		}
	}
}