#pragma once
#include "../JSScript.hpp"
#include "../common.hpp"
#include <home-scripting/main/RoomView.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            /// @brief JavaScript Room wrapper
            /// 
            class JSRoom
            {
              private:
                static duk_ret_t Constructor(duk_context* context);

                static duk_ret_t IsValid(duk_context* context);

                static duk_ret_t GetName(duk_context* context);
                static duk_ret_t SetName(duk_context* context);

              public:
                static bool Import(duk_context* context);

                static bool New(duk_context* context, Ref<RoomView> roomView);
            };
        }
    }
}