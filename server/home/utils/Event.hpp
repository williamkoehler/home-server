#pragma once
#include "../common.hpp"

namespace home
{
	template<class Type = void, class Self = void>
	using EventCallback = bool(Self*, Ref<Type>);

	class Event
	{
	protected:
		/// @brief Invoke event callback
		virtual void Invoke() = 0;
	};
}