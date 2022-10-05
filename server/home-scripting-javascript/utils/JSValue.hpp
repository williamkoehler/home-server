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
            class JSValue
            {
              private:
                static duk_ret_t duk_endpoint_constructor(duk_context* context);
                static duk_ret_t duk_color_constructor(duk_context* context);

                static duk_ret_t duk_get_value_type(duk_context* context);

              public:
                static bool duk_import(duk_context* context);

                static void duk_new_value(duk_context* context, Ref<Value> value);
                static bool duk_new_endpoint(duk_context* context, const Endpoint& endpoint);
                static bool duk_new_color(duk_context* context, const Color& color);

                static Ref<Value> duk_get_value(duk_context* context, duk_idx_t idx);
                static void duk_get_value(duk_context* context, duk_idx_t idx, Ref<Value> value);
            };
        }
    }
}