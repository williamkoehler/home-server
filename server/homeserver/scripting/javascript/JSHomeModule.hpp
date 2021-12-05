#pragma once
#include "../../common.hpp"
#include "duktape.h"

namespace server
{
	class Home;
	class Room;
	class Device;
	class DeviceController;
	class Action;

	namespace javascript
	{
		class JSHomeModule
		{
		public:
			static bool Import(duk_context* context);
		};

		class JSRoom
		{
		private:
			friend class JSHomeModule;

			static duk_ret_t Constructor(duk_context* context);

			static duk_ret_t IsValid(duk_context* context);

			static duk_ret_t GetName(duk_context* context);
			static duk_ret_t SetName(duk_context* context);

		public:
			static bool New(duk_context* context, Ref<Room> room);
		};

		class JSDevice
		{
		private:
			friend class JSHomeModule;

			static duk_ret_t Constructor(duk_context* context);

			static duk_ret_t IsValid(duk_context* context);

			static duk_ret_t GetName(duk_context* context);
			static duk_ret_t SetName(duk_context* context);

		public:
			static bool New(duk_context* context, Ref<Device> device);
		};

		class JSDeviceController
		{
		private:
			friend class JSHomeModule;

			static duk_ret_t Constructor(duk_context* context);

			static duk_ret_t IsValid(duk_context* context);

			static duk_ret_t GetName(duk_context* context);
			static duk_ret_t SetName(duk_context* context);

		public:
			static bool New(duk_context* context, Ref<DeviceController> deviceController);
		};

		class JSAction
		{
		private:
			friend class JSHomeModule;

			static duk_ret_t Constructor(duk_context* context);

			static duk_ret_t IsValid(duk_context* context);

			static duk_ret_t GetName(duk_context* context);
			static duk_ret_t SetName(duk_context* context);

		public:
			static bool New(duk_context* context, Ref<Action> action);
		};
	}
}