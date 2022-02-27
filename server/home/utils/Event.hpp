#pragma once
#include "../common.hpp"

namespace home
{
	template<class Type = void, class Self = void>
	using EventCallback = bool(Self*, Ref<Type>);

	// Union to allow pointer conversion from method pointer to function pointer
	template<class Type = void, class Self = void>
	union EventCallbackConversion
	{
		bool (Self::*method)(Ref<Type>);
		EventCallback<Type, Self>* function;
	};

	class Event
	{
	public:
		/// @brief Invoke event callback
		virtual void Invoke() = 0;
	};
}