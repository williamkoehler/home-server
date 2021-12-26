#pragma once
#include "../common.hpp"
#include <utils/Property.hpp>
#include <utils/Timer.hpp>
#include <utils/Event.hpp>

namespace server
{
    class Scriptable
    {
    public:
        virtual bool AddAttribute(const std::string& id, const char* json) = 0;
		virtual bool RemoveAttribute(const std::string& id) = 0;
        virtual void ClearAttributes() = 0;

		virtual Ref<home::Property> AddProperty(const std::string& id, home::PropertyType type) = 0;
		virtual bool RemoveProperty(const std::string& id) = 0;
        virtual void ClearProperties() = 0;

		virtual Ref<home::Event> AddEvent(const std::string& id, const std::string& callback) = 0;
		virtual bool RemoveEvent(const std::string& id) = 0;
        virtual void ClearEvents() = 0;

		virtual Ref<home::Timer> AddTimer(const std::string& id, const std::string& callback) = 0;
		virtual bool RemoveTimer(const std::string& id) = 0;
        virtual void ClearTimers() = 0;
    };
}