#include "JSHome.hpp"

#define HOME_OBJECT ("Home")

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            const char* HomeObject = HOME_OBJECT;
            const size_t HomeObjectSize = std::size(HOME_OBJECT) - 1;
        }
    }
}