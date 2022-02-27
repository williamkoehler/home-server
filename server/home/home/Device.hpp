#pragma once
#include "../common.hpp"
#include "../utils/Property.hpp"
#include "../utils/Event.hpp"
#include "../utils/Timer.hpp"
#include "DeviceController.hpp"

namespace home
{
	class DevicePlugin;

	class Device
	{
	public:
		virtual Ref<DevicePlugin> GetPlugin() = 0;

		virtual Ref<DeviceController> GetController() = 0;

		/// @brief Add property callback to property list
		/// @param id Property id
		/// @param property Property callback (called when a property is requested or set)
		virtual bool AddAttribute(const std::string &id, const char *json) = 0;

		/// @brief Remove property callback from property list
		/// @param id Property id
		/// @return Successfulness
		virtual bool RemoveAttribute(const std::string &id) = 0;

		/// @brief Add property callback to property list
		/// @param id Property id
		/// @param property Property callback (called when a property is requested or set)
		virtual Ref<Property> AddProperty(const std::string &id, Ref<Property> property) = 0;

		/// @brief Remove property callback from property list
		/// @param id Property id
		/// @return Successfulness
		virtual bool RemoveProperty(const std::string &id) = 0;

		/// @brief Add event to event list
		/// @param id Event name
		/// @param callback Event callback (called when event is fired)
		/// @return Event
		virtual Ref<Event> AddEvent(const std::string &id, EventCallback<Device> *callback) = 0;

		/// @brief Remove event from event list
		/// @param id Event id
		/// @return Successfulness
		virtual bool RemoveEvent(const std::string &id) = 0;

		/// @brief Add event to event list
		/// @tparam T Callback function type
		/// @param id Event id
		/// @param callback Event callback (called when event is fired)
		/// @return Event
		template <typename T>
		inline Ref<Event> AddEvent(const std::string &id, bool (T::*callback)(Ref<Device>))
		{
			return AddEvent(id, (EventCallback<Device> *)(EventCallbackConversion<Device, T>{callback}).function);
		}

		/// @brief Ass timer to timer list
		/// @param id Timer id
		/// @param callback Timer callback (called when timer is fired)
		/// @param interval Interval in seconds
		/// @return Timer
		virtual Ref<Timer> AddTimer(const std::string &id, TimerCallback<Device> *callback) = 0;

		/// @brief Remove timer from timer list
		/// @param id Timer id
		/// @return Successfulness
		virtual bool RemoveTimer(const std::string &id) = 0;

		/// @brief Ass timer to timer list
		/// @tparam T Callback function type
		/// @param id Timer id
		/// @param callback Timer callback (called when timer is fired)
		/// @return Timer
		template <typename T>
		inline Ref<Timer> AddTimer(const std::string &id, bool (T::*callback)(Ref<Device>))
		{
			return AddTimer(id, (TimerCallback<Device> *)(TimerCallbackConversion<Device, T>{callback}).function);
		}
	};
}