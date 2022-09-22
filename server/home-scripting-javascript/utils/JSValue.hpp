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
            /// @brief JavaScript color
            ///
            class JSColor
            {
              private:
                static duk_ret_t duk_constructor(duk_context* context);

              public:
                static bool duk_import(duk_context* context);

                static bool duk_new_color(duk_context* context, const Color& color);
            };

            /// @brief JavaScript endpoint
            ///
            class JSEndpoint
            {
              private:
                static duk_ret_t duk_constructor(duk_context* context);

              public:
                static bool duk_import(duk_context* context);

                static bool duk_new_endpoint(duk_context* context, const Endpoint& endpoint);
            };

            class JSValue
            {
              public:
                static void duk_push_value(duk_context* context, Ref<Value> value);

                static Ref<Value> duk_get_value(duk_context* context, duk_idx_t idx);
                static void duk_get_value(duk_context* context, duk_idx_t idx, Ref<Value> value);
            };
        }
    }
}