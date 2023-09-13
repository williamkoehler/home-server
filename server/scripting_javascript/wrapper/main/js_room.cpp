#include "js_home.hpp"
#include <scripting/view/main/home_view.hpp>
#include <scripting/view/main/room_view.hpp>

#include "../literals.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            duk_ret_t duk_room_constructor(duk_context* context)
            {
                if (!duk_is_constructor_call(context))
                    return DUK_RET_ERROR;

                // Expect [ number ]
                if (duk_get_top(context) != 1 || !duk_is_number(context, -1))
                    return DUK_RET_ERROR;

                // Push this
                duk_push_this(context); // [ number this ]

                // Set unique id
                duk_dup(context, -2);                                                       // [ number this number ]
                duk_put_prop_lstring(context, -2, ROOM_ID_PROPERTY, ROOM_ID_PROPERTY_SIZE); // [ number this ]

                // Pop this and number
                duk_pop_2(context); // [ ]

                return 0;
            }

            duk_ret_t duk_room_is_valid(duk_context* context)
            {
                // Expect [ ]
                if (duk_get_top(context) == 0)
                {
                    // Get id
                    duk_push_this(context);                                                     // [ this ]
                    duk_get_prop_lstring(context, -1, ROOM_ID_PROPERTY, ROOM_ID_PROPERTY_SIZE); // [ this number ]
                    identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

                    // Pop 2
                    duk_pop_2(context); // [ ]

                    Ref<HomeView> homeView = HomeView::GetHomeView();
                    assert(homeView != nullptr);

                    // Get room view
                    Ref<RoomView> roomView = homeView->GetRoom(roomID);

                    // Push result
                    duk_push_boolean(context, roomView != nullptr);

                    return 1; // [ boolean ]
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            duk_ret_t duk_room_get_name(duk_context* context)
            {
                // Expect [ ]
                if (duk_get_top(context) == 0)
                {
                    // Get id
                    duk_push_this(context);                                                     // [this]
                    duk_get_prop_lstring(context, -1, ROOM_ID_PROPERTY, ROOM_ID_PROPERTY_SIZE); // [this number]
                    identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

                    // Pop 2
                    duk_pop_2(context); // [ ]

                    // Get home view
                    Ref<HomeView> homeView = HomeView::GetHomeView();
                    assert(homeView != nullptr);

                    // Get room view
                    Ref<RoomView> roomView = homeView->GetRoom(roomID);
                    if (roomView != nullptr)
                    {
                        std::string name = roomView->GetName();

                        // Push name
                        duk_push_lstring(context, name.data(), name.size()); // [ string ]

                        return 1; // [ string ]
                    }
                    else
                    {
                        // Push undefined
                        duk_push_undefined(context); // [ undefined ]

                        return 1; // [ undefined ]
                    }
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }
            duk_ret_t duk_room_set_name(duk_context* context)
            {
                // Expect [ string ]
                if (duk_get_top(context) == 1 && duk_is_string(context, -1))
                {
                    // Get value
                    size_t nameLength;
                    const char* name = duk_get_lstring(context, -1, &nameLength);

                    // Get id
                    duk_push_this(context); // [ string this ]
                    duk_get_prop_lstring(context, -1, ROOM_ID_PROPERTY,
                                         ROOM_ID_PROPERTY_SIZE); // [ string this number ]
                    identifier_t roomID = (identifier_t)duk_get_uint(context, -1);

                    // Get home view
                    Ref<HomeView> homeView = HomeView::GetHomeView();
                    assert(homeView != nullptr);

                    // Get room view
                    Ref<RoomView> roomView = homeView->GetRoom(roomID);
                    if (roomView != nullptr)
                    {
                        // Pop number and this
                        duk_pop_2(context); // [ string ]

                        // Set name
                        roomView->SetName(std::string(name, nameLength));

                        return 1; // [ string ]
                    }
                    else
                    {
                        // Pop number, this and string
                        duk_pop_3(context);

                        // Push undefined
                        duk_push_undefined(context); // [ undefined ]

                        return 1; // [ undefined ]
                    }
                }
                else
                {
                    // Error
                    return DUK_RET_ERROR;
                }
            }

            bool duk_new_room(duk_context* context, Ref<RoomView> roomView)
            {
                assert(context != nullptr);
                assert(roomView != nullptr);

                // New room object
                duk_get_global_lstring(context, ROOM_OBJECT, ROOM_OBJECT_SIZE); // [function]
                duk_push_uint(context, roomView->GetID());                      // [function number]
                duk_new(context, 1);                                            // [object]

                return true;
            }
        }
    }
}