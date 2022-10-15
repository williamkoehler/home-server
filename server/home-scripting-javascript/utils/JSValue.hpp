#pragma once
#include "../JSScript.hpp"
#include "../common.hpp"
#include <home-scripting/utils/Value.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            void duk_new_value(duk_context* context, const Value& value);

            bool duk_new_endpoint(duk_context* context, const Endpoint& endpoint);
            bool duk_new_color(duk_context* context, const Color& color);
            bool duk_new_room_id(duk_context* context, identifier_t id);
            bool duk_new_device_id(duk_context* context, identifier_t id);
            bool duk_new_service_id(duk_context* context, identifier_t id);

            Value duk_get_value(duk_context* context, duk_idx_t idx);
            void duk_get_value(duk_context* context, duk_idx_t idx, Value& value);

            bool duk_import_value(duk_context* context);
        }
    }
}
