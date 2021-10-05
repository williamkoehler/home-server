#pragma once
#include "../common.hpp"
#include "Home.hpp"
#include "Room.hpp"
#include <home/Home.hpp>
#include <home/Room.hpp>
#include <home/Device.hpp>
#include <home/DeviceScript.hpp>

namespace lua
{
	class LuaDevice;
}

namespace server
{
	class SignalManager;

	class Device : public home::Device, public boost::enable_shared_from_this<Device>
	{
	private:
		friend class Home;
		friend class Room;
		friend class JsonApi;
		friend class lua::LuaDevice;

		boost::shared_mutex mutex;

		size_t roomCount;

		std::string name;
		const uint32_t deviceID;
		
		boost::mutex scriptMutex;
		boost::atomic<time_t> timestamp;
		const Ref<home::DeviceScript> script;

	public:
		Device(std::string name, uint32_t deviceID, Ref<home::DeviceScript> script);
		~Device();
		static Ref<Device> Create(std::string name, uint32_t deviceID, uint32_t scriptID, rapidjson::Value& json);
		
		virtual inline Ref<home::Home> GetHome() override 
		{
			boost::shared_lock_guard lock(mutex);
			return Home::GetInstance(); 
		}
		
		virtual std::string GetName() override
		{
			boost::shared_lock_guard lock(mutex);
			return name;
		}
		virtual void SetName(std::string v) override
		{
			boost::lock_guard lock(mutex);
			Home::GetInstance()->UpdateTimestamp();
			name = std::move(v);
		}

		virtual inline uint32_t GetDeviceID() override { return deviceID; }

		/// @brief Add device to update list
		virtual void AddUpdatable() override;

		virtual inline Ref<home::DeviceScript> GetScript() override { return script; }
		virtual inline uint32_t GetScriptID() override { return script->GetScriptID(); }

		/// @brief Update device plugin
		/// @param signalManager SignalManager
		/// @param cycle Current singal cycle
		void Update(Ref<SignalManager> signalManager, size_t cycle);

		/// @brief Save device to JSON
		/// @param json JSON
		/// @param allocator JSON allocator 
		void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
	};
}