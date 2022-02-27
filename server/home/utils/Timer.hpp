#pragma once
#include "../common.hpp"

namespace home
{
	template<class Type = void, class Self = void>
	using TimerCallback = bool(Self*, Ref<Type>);

	// Union to allow pointer conversion from method pointer to function pointer
	template<class Type = void, class Self = void>
	union TimerCallbackConversion
	{
		bool (Self::*method)(Ref<Type>);
		TimerCallback<Type, Self>* function;
	};

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