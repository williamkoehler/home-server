#include "JSScript.hpp"
#include "../ScriptSource.hpp"
#include "../../tools.hpp"

extern "C"
{
	// Interrupt function called by the duktape engine
	// This allows to interruption of any javascript code that runs too long
	duk_ret_t duk_exec_timeout(void* udata)
	{
		return ((server::javascript::JSScript*)udata)->CheckTimeout();
	}
}

namespace server
{
	namespace javascript
	{
		JSScript::JSScript(Ref<ScriptSource> source) : Script(source),
			context(nullptr)
		{ }
		JSScript::~JSScript()
		{
		}

		void JSScript::PrepareTimeout(size_t maxTime)
		{
			startTime = clock() / (CLOCKS_PER_SEC / 1000);
			maxTime = maxTime;
		}

		duk_ret_t JSScript::PrepareSafe(duk_context* context, void* udata)
		{
			JSScript* script = (JSScript*)udata;

			// Setup default variables
			duk_push_object(context);
			duk_put_global_lstring(context, "events", 6);

			// Import modules
			//script->ImportModules();

			// Load script source
			std::string data = script->source->GetData();
			duk_push_lstring(context, (const char*)data.c_str(), data.size());

			std::string name = script->source->GetName();
			duk_push_lstring(context, (const char*)name.c_str(), name.size());

			// Compile
			duk_compile(context, 0);

			// Call script
			duk_call(context, 0);
			duk_pop(context);

			// Prepare properties
			duk_get_global_string(context, "properties");

			// Iterate over every property in 'properties'
			duk_enum(context, -1, 0);
			while (duk_next(context, -1, 0))
			{
				size_t nameLength;
				const char* nameStr = duk_to_lstring(context, -1, &nameLength);
				std::string name = std::string(nameStr, nameLength);

				// Read type
				duk_dup_top(context);
				duk_get_prop(context, -4);

				Ref<home::Property> property;

				size_t typeLength;
				const char* typeStr = duk_to_lstring(context, -1, &typeLength);
				switch (crc32(typeStr, typeLength))
				{
				case CRC32("boolean"):
					property = home::BooleanProperty::Create();
					break;
				case CRC32("integer"):
					property = home::IntegerProperty::Create();
					break;
				case CRC32("number"):
					property = home::NumberProperty::Create();
					break;
				case CRC32("string"):
					property = home::StringProperty::Create();
					break;
				case CRC32("endpoint"):
					property = home::EndpointProperty::Create();
					break;
				case CRC32("color"):
					property = home::ColorProperty::Create();
					break;
				default:
					property = home::NullProperty::Create();
					break;
				}

				// Insert
				uint32_t index = (uint32_t)script->propertyListByID.size();
				script->propertyList[name] = property;
				script->propertyListByID.push_back(property);

				// Pop type
				duk_pop(context);

				// Push function
				duk_push_c_function(context, JSScript::PropertyGetter, 0);
				duk_set_magic(context, -1, index);
				duk_push_c_function(context, JSScript::PropertySetter, 1);
				duk_set_magic(context, -1, index);

				duk_def_prop(context, -5,
					DUK_DEFPROP_HAVE_GETTER |
					DUK_DEFPROP_HAVE_SETTER |
					DUK_DEFPROP_FORCE |
					DUK_DEFPROP_HAVE_ENUMERABLE |
					DUK_DEFPROP_HAVE_CONFIGURABLE);
			}

			// Pop properties
			duk_pop(context);

			return DUK_EXEC_SUCCESS;
		}

		duk_ret_t JSScript::InvokeSafe(duk_context* context, void* udata)
		{
			JSScript* script = (JSScript*)duk_get_user_data(context);

			const std::string* event = (const std::string*)udata;

			// Get events object
			if (!duk_get_global_lstring(context, "events", 6))
				return DUK_RET_ERROR;

			// Get event function
			if (!duk_get_prop_lstring(context, -1, event->data(), event->size()))
				return DUK_RET_ERROR;

			// Prepare timout
			script->PrepareTimeout(5000); // 5 seconds

			// Execute event
			duk_call(context, 0);
			duk_pop(context);

			return DUK_EXEC_SUCCESS;
		}

		duk_ret_t JSScript::PropertyGetter(duk_context* context)
		{
			JSScript* script = (JSScript*)duk_get_user_data(context);
			uint32_t index = duk_get_current_magic(context);

			if (index < script->propertyListByID.size())
			{
				Ref<home::Property> property = script->propertyListByID[index];
				assert(property != nullptr);

				switch (property->GetType())
				{
				case home::PropertyType::kBooleanType:
					duk_push_boolean(context, property->GetBoolean());
					return 1;
				case home::PropertyType::kIntegerType:
					duk_push_int(context, property->GetInteger());
					return 1;
				case home::PropertyType::kNumberType:
					duk_push_number(context, property->GetNumber());
					return 1;
				case home::PropertyType::kStringType:
				{
					std::string string = property->GetString();
					duk_push_lstring(context, string.data(), string.size());
					return 1;
				}
				case home::PropertyType::kEndpointType:
				{
					duk_push_object(context);

					home::Endpoint endpoint = property->GetEndpoint();

					duk_push_lstring(context, endpoint.host.data(), endpoint.host.size());
					duk_put_prop_lstring(context, -2, "host", 4);

					duk_push_int(context, endpoint.port);
					duk_put_prop_lstring(context, -2, "port", 4);

					return 1;
				}
				case home::PropertyType::kColorType:
				{
					duk_push_object(context);

					home::Color color = property->GetColor();

					duk_push_int(context, color.red);
					duk_put_prop_lstring(context, -2, "red", 3);

					duk_push_int(context, color.green);
					duk_put_prop_lstring(context, -2, "green", 5);

					duk_push_int(context, color.blue);
					duk_put_prop_lstring(context, -2, "blue", 4);

					return 1;
				}
				}
			}

			// Error
			duk_push_null(context);
			return 1;
		}
		duk_ret_t JSScript::PropertySetter(duk_context* context)
		{
			JSScript* script = (JSScript*)duk_get_user_data(context);
			uint32_t index = duk_get_current_magic(context);

			if (index < script->propertyListByID.size() && duk_get_top(context) == 1)
			{
				Ref<home::Property> property = script->propertyListByID[index];
				assert(property != nullptr);

				switch (property->GetType())
				{
				case home::PropertyType::kBooleanType:
				{
					property->SetBoolean(duk_to_boolean(context, -1));
					duk_pop(context);

					return 0;
				}
				case home::PropertyType::kIntegerType:
				{
					property->SetInteger(duk_to_int(context, -1));
					duk_pop(context);

					return 0;
				}
				case home::PropertyType::kNumberType:
				{
					property->SetNumber(duk_to_number(context, -1));
					duk_pop(context);

					return 0;
				}
				case home::PropertyType::kStringType:
				{
					size_t length;
					const char* string = duk_to_lstring(context, -1, &length);
					property->SetString(std::string(string, length));
					duk_pop(context);

					return 0;
				}
				case home::PropertyType::kEndpointType:
				{
					// Coerce value
					duk_to_object(context, -1);

					// Get each property
					duk_get_prop_lstring(context, -1, "host", 4);
					duk_get_prop_lstring(context, -2, "port", 4);

					size_t hostLength;
					const char* host = duk_to_lstring(context, -2, &hostLength);

					home::Endpoint endpoint;
					endpoint.host = std::string(host, hostLength);
					endpoint.port = (uint16_t)duk_to_int(context, -1);

					property->SetEndpoint(endpoint);

					duk_pop_3(context);

					return 0;
				}
				case home::PropertyType::kColorType:
				{
					// Coerce value
					duk_to_object(context, -1);

					// Get each property
					duk_get_prop_lstring(context, -1, "red", 3);
					duk_get_prop_lstring(context, -2, "green", 5);
					duk_get_prop_lstring(context, -3, "blue", 4);

					home::Color color;
					color.red = (uint8_t)duk_to_int(context, -3);
					color.green = (uint8_t)duk_to_int(context, -2);
					color.blue = (uint8_t)duk_to_int(context, -1);

					property->SetColor(color);

					duk_pop_n(context, 4);

					return 0;
				}
				}
			}

			// Error
			return 0;
		}

		bool JSScript::Invoke(const std::string& event)
		{
			// Check if compilation stage is needed
			size_t c = source->GetChecksum();
			if (c != checksum || context == nullptr)
			{
				context = Ref<duk_context>(duk_create_heap(nullptr, nullptr, nullptr, this, nullptr), [](duk_context* context) -> void { duk_destroy_heap(context); });
				if (context == nullptr)
				{
					checksum = 0;
					return false;
				}

				// Prepare context
				if (duk_safe_call(context.get(), JSScript::PrepareSafe, this, 0, 0) != 0)
				{
					//TODO Error message duk_safe_to_string(context, -1);

					context = nullptr;
					checksum = 0;
					return false;
				}

				// Set checksum to prevent recompilation
				checksum = c;
			}

			if (duk_safe_call(context.get(), JSScript::InvokeSafe, (void*)&event, 0, 0) != 0)
			{
				//TODO Error message duk_safe_to_string(context, -1);

				context = nullptr;
				return false;
			}

			return true;
		}
	}
}