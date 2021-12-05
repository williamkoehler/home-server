#pragma once
#include "../common.hpp"

namespace home
{
	template<class Type = void, class Self = void>
	using TimerCallback = bool(Self*, Ref<Type>);

	class Timer
	{
	protected:
		/// @brief Invoke timer callback
		virtual void Invoke() = 0;

	public:

		/// @brief Start timer
		/// @param interval Interval in seconds
		virtual void Start(size_t interval) = 0;

		/// @brief Stop timer
		virtual void Stop() = 0;
	};
}