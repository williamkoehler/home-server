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
                static duk_ret_t duk_constructor(duk_context* context);

                static duk_ret_t duk_is_valid(duk_context* context);

                static duk_ret_t duk_get_name(duk_context* context);
                static duk_ret_t duk_set_name(duk_context* context);

              public:
                static bool duk_import(duk_context* context);

                static bool duk_new_room(duk_context* context, Ref<RoomView> roomView);
            };
        }
    }
}