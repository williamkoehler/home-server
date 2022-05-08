#include "JSUtils.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            bool JSUtils::Import(duk_context* context)
            {
                assert(context != nullptr);

                // Import endpoint
                {
                    // duk_push_c_function(context, )
                }

                return true;
            }

            void JSUtils::GetProperty(duk_context* context, Ref<Property> property)
            {
                switch (property->GetType())
                {
                case PropertyType::kBooleanType:
                    duk_push_boolean(context, property->GetBoolean());
                    break;
                case PropertyType::kIntegerType:
                    duk_push_int(context, property->GetInteger());
                    break;
                case PropertyType::kNumberType:
                    duk_push_number(context, property->GetNumber());
                    break;
                case PropertyType::kStringType: {
                    std::string value = property->GetString();
                    duk_push_lstring(context, value.data(), value.size());

                    break;
                }
                case PropertyType::kEndpointType: {
                    Endpoint endpoint = property->GetEndpoint();

                    duk_push_object(context);

                    // Push _class
                    duk_push_lstring(context, "endpoint", 8);
                    duk_put_prop_lstring(context, -2, "_class", 6);

                    // Push host
                    duk_push_lstring(context, endpoint.host.data(), endpoint.host.size());
                    duk_put_prop_lstring(context, -2, "host", 4);

                    // Push port
                    duk_push_int(context, endpoint.port);
                    duk_put_prop_lstring(context, -2, "port", 4);

                    break;
                }
                case PropertyType::kColorType: {
                    Color color = property->GetColor();

                    duk_push_object(context);

                    // Push _class
                    duk_push_lstring(context, "color", 5);
                    duk_put_prop_lstring(context, -2, "_class", 6);

                    // Push red
                    duk_push_int(context, color.red);
                    duk_put_prop_lstring(context, -2, "red", 3);

                    // Push green
                    duk_push_int(context, color.green);
                    duk_put_prop_lstring(context, -2, "green", 5);

                    // Push blue
                    duk_push_int(context, color.blue);
                    duk_put_prop_lstring(context, -2, "blue", 4);

                    break;
                }
                default: {
                    // Error or unknown type
                    duk_push_null(context);

                    break;
                }
                }
            }

            void JSUtils::SetProperty(duk_context* context, Ref<Property> property)
            {
                switch (duk_get_type(context, -1))
                {
                case DUK_TYPE_BOOLEAN:
                    property->SetBoolean(duk_get_boolean(context, -1));
                    break;
                case DUK_TYPE_NUMBER:
                    property->SetNumber(duk_get_number(context, -1));
                    break;
                case DUK_TYPE_STRING: {
                    size_t valueLength;
                    const char* value = duk_get_lstring(context, -1, &valueLength);
                    property->SetString(std::string(value, valueLength));

                    break;
                }
                case DUK_TYPE_OBJECT: {

                    break;
                }
                }
            }
        }
    }
}