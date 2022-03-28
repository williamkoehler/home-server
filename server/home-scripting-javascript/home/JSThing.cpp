// #include "JSThing.hpp"
// #include "../../../home/Home.hpp"

// namespace server
// {
//     namespace javascript
//     {
//         bool JSThing::Import(duk_context* context)
//         {
//             assert(context != nullptr);

//             duk_push_c_function(context, JSThing::Constructor, 1);
//             duk_push_object(context); // Prototype

//             // Register methods
//             static const duk_function_list_entry methods[] = {{"isValid", JSThing::IsValid, 0},
//                                                               {"getName", JSThing::GetName, 0},
//                                                               {"setName", JSThing::SetName, 1},
//                                                               {nullptr, nullptr, 0}};

//             duk_put_function_list(context, -1, methods);

//             // Register prototype and constructor
//             duk_put_prop_string(context, -2, "prototype");
//             duk_put_global_string(context, THING_OBJECT);

//             return true;
//         }

//         duk_ret_t JSThing::Constructor(duk_context* context)
//         {
//             if (!duk_is_constructor_call(context))
//                 return DUK_RET_ERROR;

//             // Expect [integer]
//             if (duk_get_top(context) != 1 || !duk_is_number(context, -1))
//                 return DUK_RET_ERROR;

//             // Get this
//             duk_push_this(context); // [number this]

//             // Set integer
//             duk_dup(context, 0);                         // [number this number]
//             duk_put_prop_string(context, -2, UNIQUE_ID); // [number this]

//             return 0;
//         }

//         duk_ret_t JSThing::IsValid(duk_context* context)
//         {
//             // Expect []
//             if (duk_get_top(context) != 0)
//                 return DUK_RET_ERROR;

//             // Get id
//             duk_push_this(context);                      // [this]
//             duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
//             identifier_t id = (identifier_t)duk_get_uint(context, -1);

//             // Pop 2
//             duk_pop_2(context); // []

//             // Get device
//             Ref<Home> home = Home::GetInstance();
//             assert(home != nullptr);

//             Ref<Thing> thing = home->GetThing(id);
//             duk_push_boolean(context, thing != nullptr);

//             return 1;
//         }

//         duk_ret_t JSThing::GetName(duk_context* context)
//         {
//             // Expect []
//             if (duk_get_top(context) != 0)
//                 return DUK_RET_ERROR;

//             // Get id
//             duk_push_this(context);                      // [this]
//             duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
//             identifier_t id = (identifier_t)duk_get_uint(context, -1);

//             // Get device
//             Ref<Home> home = Home::GetInstance();
//             assert(home != nullptr);

//             Ref<Thing> thing = home->GetThing(id);
//             if (thing == nullptr)
//                 return DUK_RET_ERROR;

//             // Pop 2
//             duk_pop_2(context); // []

//             // Get name
//             std::string name = thing->GetName();
//             duk_push_lstring(context, name.data(), name.size()); // [string]

//             return 1;
//         }
//         duk_ret_t JSThing::SetName(duk_context* context)
//         {
//             // Expect [string]
//             if (duk_get_top(context) != 1 || !duk_is_string(context, -1))
//                 return DUK_RET_ERROR;

//             // Get value
//             size_t valueLength;
//             const char* value = duk_get_lstring(context, -1, &valueLength);

//             // Get id
//             duk_push_this(context);                      // [string this]
//             duk_get_prop_string(context, -1, UNIQUE_ID); // [string this number]
//             identifier_t id = (identifier_t)duk_get_uint(context, -1);

//             // Get device
//             Ref<Home> home = Home::GetInstance();
//             assert(home != nullptr);

//             Ref<Thing> thing = home->GetThing(id);
//             if (thing == nullptr)
//                 return DUK_RET_ERROR;

//             // Set name
//             thing->SetName(std::string(value, valueLength));

//             // Pop 3
//             duk_pop_3(context); // []

//             return 0;
//         }

//         bool JSThing::New(duk_context* context, Ref<Thing> thing)
//         {
//             assert(context != nullptr);
//             assert(thing != nullptr);

//             // New device object
//             duk_get_global_string(context, THING_OBJECT); // [function]
//             duk_push_uint(context, thing->GetID());       // [function number]
//             duk_new(context, 1);                          // [object]

//             return true;
//         }
//     }
// }