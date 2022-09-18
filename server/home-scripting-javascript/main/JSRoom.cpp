#include "JSRoom.hpp"
#include <home-scripting/ScriptManager.hpp>
#include <home-scripting/main/HomeView.hpp>
#include <home-scripting/main/RoomView.hpp>

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            bool JSRoom::Import(duk_context* context)
            {
                assert(context != nullptr);

                duk_push_c_function(context, JSRoom::Constructor, 1); // [ c_func ]
                duk_push_object(context);                             // [ c_func object ]

                // Register methods
                static const duk_function_list_entry methods[] = {
                    {"isValid", JSRoom::IsValid, 0},
                    {"getName", JSRoom::GetName, 0},
                    {"setName", JSRoom::SetName, 1},
                    {nullptr, nullptr, 0},
                };

                duk_put_function_list(context, -1, methods); // [ c_func object ]

                // Register prototype and constructor
                duk_put_prop_lstring(context, -2, "prototype", 9);
                duk_put_global_string(context, ROOM_OBJECT);

                return true;
            }

            duk_ret_t JSRoom::Constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ number ]
                if (duk_get_top(context) != 1 || !duk_is_number(context, -1))
                    return DUK_RET_ERROR;

                // Push this
                duk_push_this(context); // [ number this ]

                // Set unique id
                duk_dup(context, -2);                        // [ number this number ]
                duk_put_prop_string(context, -2, UNIQUE_ID); // [ number this ]

                // Pop number and this
                duk_pop_2(context);

                return 0;
            }

            duk_ret_t JSRoom::IsValid(duk_context* context)
            {
                // Expect [ ]
                if (duk_get_top(context) == 0)
                {
                    // Get id
                    duk_push_this(context);                      // [ this ]
                    duk_get_prop_string(context, -1, UNIQUE_ID); // [ this number ]
                    identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

                    // Pop 2
                    duk_pop_2(context); // [ ]

                    // Get home view
                    // Ref<HomeView> homeView = ScriptManager::GetHomeView();
                    // assert(homeView != nullptr);

                    // // Get room view
                    // Ref<RoomView> roomView = homeView->GetRoom(roomID);

                    // // Push result
                    // duk_push_boolean(context, roomView != nullptr);
                    return 1;
                }

                // Push undefined
                duk_push_undefined(context);
                return 1;
            }

            duk_ret_t JSRoom::GetName(duk_context* context)
            {
                // Expect [ ]
                if (duk_get_top(context) == 0)
                {
                    // Get id
                    duk_push_this(context);                      // [this]
                    duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
                    identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

                    // Pop 2
                    duk_pop_2(context); // [ ]

                    // Get home view
                    // Ref<HomeView> homeView = ScriptManager::GetHomeView();
                    // assert(homeView != nullptr);

                    // // Get room view
                    // Ref<RoomView> roomView = homeView->GetRoom(roomID);
                    // if (roomView != nullptr)
                    // {
                    //     std::string name = roomView->GetName();

                    //     // Push name
                    //     duk_push_lstring(context, name.data(), name.size()); // [ string ]
                    //     return 1;
                    // }
                }

                // Push undefined
                duk_push_undefined(context);
                return 1;
            }
            duk_ret_t JSRoom::SetName(duk_context* context)
            {
                // Expect [ string ]
                if (duk_get_top(context) == 1 && duk_is_string(context, -1))
                {
                    // Get value
                    size_t nameLength;
                    const char* name = duk_get_lstring(context, -1, &nameLength);

                    // Get id
                    duk_push_this(context);                      // [ string this ]
                    duk_get_prop_string(context, -1, UNIQUE_ID); // [ string this number ]
                    identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

                    // Get home view
                    // Ref<HomeView> homeView = ScriptManager::GetHomeView();
                    // assert(homeView != nullptr);

                    // // Get room view
                    // Ref<RoomView> roomView = homeView->GetRoom(roomID);
                    // if (roomView == nullptr)
                    //     return DUK_RET_ERROR;

                    // // Set name
                    // roomView->SetName(std::string(name, nameLength));
                }

                return 0;
            }

            bool JSRoom::New(duk_context* context, Ref<RoomView> roomView)
            {
                assert(context != nullptr);
                assert(roomView != nullptr);

                // New room object
                duk_get_global_string(context, ROOM_OBJECT); // [function]
                duk_push_uint(context, roomView->GetID());   // [function number]
                duk_new(context, 1);                         // [object]

                return true;
            }
        }
    }
}