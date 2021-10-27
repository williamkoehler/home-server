#pragma once
#include "../../common.hpp"
#include "duktape.h"

namespace server
{
	class Home;
	class Device;
	class Room;

	namespace javascript
	{
		class HomeModule
		{
		public:
			static bool Import(duk_context* context);
		};

		class Home
		{
		private:
			friend class HomeModule;

			static duk_ret_t Constructor(duk_context* context);

			static duk_ret_t GetDevice(duk_context* context);

		public:
			static bool New(duk_context* context, Ref<server::Home> home);
		};

		class Device
		{
		private:
			friend class HomeModule;

			static duk_ret_t Constructor(duk_context* context);

			static duk_ret_t GetDeviceName(duk_context* context);
			static duk_ret_t SetDeviceName(duk_context* context);

		public:
			static bool New(duk_context* context, Ref<server::Device> device);
		};
	}
}