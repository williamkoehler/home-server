// #pragma once
// #include "../../../common.hpp"
// #include "../../../home/Room.hpp"
// #include "../JSScript.hpp"

// namespace server
// {
//     namespace javascript
//     {
//         class JSRoom
//         {
//           private:
//             static duk_ret_t Constructor(duk_context* context);

//             static duk_ret_t IsValid(duk_context* context);

//             static duk_ret_t GetName(duk_context* context);
//             static duk_ret_t SetName(duk_context* context);

//           public:
//             static bool Import(duk_context* context);

//             static bool New(duk_context* context, Ref<Room> room);
//         };
//     }
// }