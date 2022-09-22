#pragma once
#include "../common.hpp"
#include "../JSScript.hpp"
#include <home-scripting/main/DeviceView.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            /// @brief JavaScript Device wrapper
            ///
            class JSDevice
            {
              private:
                static duk_ret_t duk_constructor(duk_context* context);

                static duk_ret_t duk_is_valid(duk_context* context);

                static duk_ret_t duk_get_name(duk_context* context);
                static duk_ret_t duk_set_name(duk_context* context);

                static duk_ret_t duk_invoke(duk_context* context);

              public:
                static bool duk_import(duk_context* context);

                static bool duk_new_device(duk_context* context, Ref<DeviceView> deviceView);
            };
        }
    }
}