#include "JSActionModule.hpp"
#include "JSScript.hpp"
#include "JSConstants.hpp"
#include "../../home/Home.hpp"
#include "../../home/Action.hpp"

namespace server
{
    namespace javascript
    {
        bool JSActionModule::Import(duk_context *context, identifier_t actionID)
        {
            assert(context != nullptr);

            // Import action object
            {
                duk_push_object(context);

                // Register methods
                static const duk_function_list_entry methods[] = {
                    {"isValid", JSActionSelf::IsValid, 0},
                    {"getName", JSActionSelf::GetName, 0},
                    {"setName", JSActionSelf::SetName, 1},
                    {nullptr, nullptr, 0}};

                duk_put_function_list(context, -1, methods);

                // Set integer
                duk_push_uint(context, actionID);
                duk_put_prop_string(context, -1, UNIQUE_ID);

                // Register action
                duk_put_global_lstring(context, "action", 6);
            }
        }

        duk_ret_t JSActionSelf::IsValid(duk_context *context)
        {
            // Expect []
            if (duk_get_top(context) != 0)
                return DUK_RET_ERROR;

            // Get id
            duk_push_this(context);                      // [this]
            duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
            identifier_t actionID = (identifier_t)duk_get_uint(context, -1);

            // Pop 2
            duk_pop_2(context); // []

            // Get room
            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            Ref<Action> action = home->GetAction(actionID);
            duk_push_boolean(context, action != nullptr);

            return 1;
        }

        duk_ret_t JSActionSelf::GetName(duk_context *context)
        {
            // Expect []
            if (duk_get_top(context) != 0)
                return DUK_RET_ERROR;

            // Get id
            duk_push_this(context);                      // [this]
            duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
            identifier_t actionID = (identifier_t)duk_get_uint(context, -1);

            // Get action
            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            Ref<Action> action = home->GetAction(actionID);
            if (action == nullptr)
                return DUK_RET_ERROR;

            // Pop 2
            duk_pop_2(context); // []

            // Get name
            std::string name = action->GetName();
            duk_push_lstring(context, name.data(), name.size()); // [string]

            return 1;
        }
        duk_ret_t JSActionSelf::SetName(duk_context *context)
        {
            // Expect [string]
            if (duk_get_top(context) != 1 || !duk_is_string(context, -1))
                return DUK_RET_ERROR;

            // Get value
            size_t valueLength;
            const char *value = duk_get_lstring(context, -1, &valueLength);

            // Get id
            duk_push_this(context);                      // [this]
            duk_get_prop_string(context, -1, UNIQUE_ID); // [this number]
            identifier_t actionID = (identifier_t)duk_get_uint(context, -1);

            // Get action
            Ref<Home> home = Home::GetInstance();
            assert(home != nullptr);

            Ref<Action> action = home->GetAction(actionID);
            if (action == nullptr)
                return DUK_RET_ERROR;

            // Set name
            action->SetName(std::string(value, valueLength));

            // Pop 3
            duk_pop_3(context); // []

            return 0;
        }
    }
}