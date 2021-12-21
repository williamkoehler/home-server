#pragma once
#include "../common.hpp"
#include "ScriptSource.hpp"
#include <utils/Property.hpp>
#include <utils/Timer.hpp>
#include <utils/Event.hpp>

namespace server
{
	class JsonApi;

	class Script : public boost::enable_shared_from_this<Script>
	{
	protected:
		friend class JsonApi;

		Ref<ScriptSource> source;
		uint64_t checksum;

		robin_hood::unordered_node_map<std::string, Ref<home::Property>> propertyList;
		robin_hood::unordered_node_map<std::string, Ref<home::Timer>> timerList;
		robin_hood::unordered_node_map<std::string, Ref<home::Event>> eventList;

	public:
		Script(Ref<ScriptSource> source);
		~Script();

		inline identifier_t GetSourceID() { return source->GetSourceID(); }

		/// @brief Compile script if necessary
		/// @return Successfulness
		virtual bool Prepare() = 0;

		/// @brief Invoke script event
		/// @param event Event name
		/// @return Successfulness
		virtual bool Invoke(const std::string& event) = 0;
	};
}