#pragma once
#include "../common.hpp"
#include <home/Home.hpp>
#include <home/Room.hpp>
#include <home/Device.hpp>
#include <home/DeviceManager.hpp>
#include <home/DeviceManagerScript.hpp>

namespace server
{
	class Device;

	class JsonApi;
	class SSHSession;

	class DeviceManager : public boost::enable_shared_from_this<DeviceManager>, public home::DeviceManager
	{
	private:
		friend class JsonApi;
		friend class SSHSession;
		friend class Device;

		boost::shared_mutex mutex;

		std::string name;
		const uint32_t managerID;

		boost::atomic<time_t> timestamp;

		boost::mutex scriptMutex;
		const Ref<home::DeviceManagerScript> script;
		
	public:
		DeviceManager(std::string name, uint32_t managerID, Ref<home::DeviceManagerScript> script);
		~DeviceManager();
		static Ref<DeviceManager> Create(std::string name, uint32_t managerID, uint32_t scriptID, rapidjson::Value& json);

		virtual inline Ref<home::Home> GetHome() override
		{
			boost::shared_lock_guard lock(mutex);
			return Home::GetInstance();
		}

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

		virtual inline uint32_t GetDeviceManagerID() override { return managerID; }

		/// @brief Add device manager to update list
		virtual void AddUpdatable() override;

		virtual inline  Ref<home::DeviceManagerScript> GetScript() override { return script; }
		virtual inline uint32_t GetScriptID() override { return script->GetScriptID(); }

		/// @brief Update device manager plugin		
		/// @param signalManager SignalManager
		/// @param cycle Current singal cycle
		void Update(Ref<SignalManager> signalManager, size_t cycle);

		/// @brief Save device manager to JSON
		/// @param json JSON
		/// @param allocator JSON allocator 
		void Save(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator);
	};
}