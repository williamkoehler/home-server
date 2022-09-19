#pragma once
#include "common.hpp"
#include "duktape.h"
#include <home-scripting/utils/Value.hpp>

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
                static bool Import(duk_context* context);

                /// @brief Push property value to stack
                ///
                /// @param context Duktape context
                /// @param property Property
                static void GetProperty(duk_context* context, Ref<Value> property);

                /// @brief Set property value from stack
                ///
                /// @param context Duktape context
                /// @param property Property
                static void SetProperty(duk_context* context, Ref<Value> property);
            };

            class JSEndpoint
            {
              private:
                friend class JSUtils;

                static duk_ret_t Constructor(duk_context* context);
            };
        }
    }
}