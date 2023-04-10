#pragma once
#include "common.hpp"

namespace server
{
    namespace scripting
    {
        enum PropertyFlags : uint8_t
        {
            /// @brief Is property visible when reading properties
            ///
            kPropertyFlag_Visible = 0x01,

            /// @brief Is property stored when saving script data
            ///
            kPropertyFlag_Store = 0x02,

            /// @brief Should property initiate update when it is changed
            ///
            kPropertyFlag_InitiateUpdate = 0x03,

            /// @brief Enable all property flags
            ///
            kPropertyFlags_All = 0xFF,
        };
    }
}