#pragma once
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            class JSUtils
            {
              public:
                /// @brief Import default structures and functions
                ///
                /// @param context
                /// @return true Successful
                /// @return false Failure
                static bool duk_import(duk_context* context);

                /// @brief Print text to terminal
                ///
                static duk_ret_t duk_print(duk_context* context);

                /// @brief Create timer
                ///
                static duk_ret_t duk_create_timer(duk_context* context);

                /// @brief Create delay
                ///
                static duk_ret_t duk_create_delay(duk_context* context);
            };
        }
    }
}