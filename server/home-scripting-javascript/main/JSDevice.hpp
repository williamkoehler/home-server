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
            /// @brief JavaScript device wrapper
            ///
            class JSDevice
            {
              private:
                friend class JSHomeModule;

                static duk_ret_t Constructor(duk_context* context);

                static duk_ret_t IsValid(duk_context* context);

                static duk_ret_t GetName(duk_context* context);
                static duk_ret_t SetName(duk_context* context);

              public:
                static bool Import(duk_context* context);

                static bool New(duk_context* context, Ref<DeviceView> deviceView);
            };
        }
    }
}