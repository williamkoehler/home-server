#pragma once
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            /// @brief Import default structures and functions
            ///
            /// @param context
            /// @return Successfulness
            bool duk_import_utils(duk_context* context);
        }
    }
}