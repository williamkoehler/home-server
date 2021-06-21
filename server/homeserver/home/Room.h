#pragma once
#include "../common.h"
#include "Home.h"
#include <home/Home.h>
#include <home/Room.h>
#include <home/Device.h>

namespace server
{
	class Core;
	class Device;
	class Action;

	class JsonApi;
	class SSHSession;

	class Room : public home::Room, public boost::enable_shared_from_this<Room>
	{
	private:
		friend class Core;
		friend class Home;
		friend class JsonApi;
		friend class SSHSession;

		boost::shared_mutex mutex;

		std::string name;
		const uint32_t roomID;
		uint32_t type;

		boost::container::set<WeakRef<Device>> deviceList;
		boost::container::set<WeakRef<Action>> actionList;
		
	public:
		Room(std::string name, uint32_t roomID, uint32_t type);
		~Room();
		static Ref<Room> Create(std::string name, uint32_t roomID, uint32_t type);
		
		boost::shared_mutex& GetMutex() { return mutex; }

		virtual inline Ref<home::Home> GetHome() override { return Home::GetInstance(); }

		inline std::string GetName()
		{
			boost::shared_lock_guard lock(mutex);
			return name;
		}
		inline void SetName(std::string v)
		{
			boost::lock_guard lock(mutex);
			Home::GetInstance()->UpdateTimestamp();
			name = std::move(v);
		}

		inline uint32_t GetRoomID() { return roomID; }

		inline uint32_t GetType()
		{
			boost::shared_lock_guard lock(mutex);
			return type;
		}
		inline void SetType(uint32_t v)
		{
			boost::lock_guard lock(mutex);
			Home::GetInstance()->UpdateTimestamp();
			type = v;
		}

		//Device
		void AddDevice(Ref<Device> device);
		virtual inline size_t GetDeviceCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return deviceList.size();
		}

		void RemoveDevice(Ref<Device> device);

		// Action
		void AddAction(Ref<Action> action);
		virtual inline size_t GetActionCount() override
		{
			boost::shared_lock_guard lock(mutex);
			return actionList.size();
		}

		void RemoveAction(Ref<Action> action);

		//IO
		void Load(rapidjson::Value& json);
		void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
	};
}