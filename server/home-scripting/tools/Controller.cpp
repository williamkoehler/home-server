#include "Controller.hpp"

namespace server
{
    namespace scripting
    {
        Controller::Controller(const Ref<Script>& script) : script(script)
        {
            assert(script != nullptr);
        }
        Controller::~Controller()
        {
        }
    }
}