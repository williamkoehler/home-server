#pragma once
#include "../../JSScript.hpp"
#include "../../common.hpp"
#include <home-scripting/view/main/HomeView.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            bool duk_new_home(duk_context* context);
            bool duk_new_room(duk_context* context, Ref<RoomView> roomView);
            bool duk_new_device(duk_context* context, Ref<DeviceView> deviceView);

            bool duk_import_home(duk_context* context);
        }
    }
}