#include "Controller.hpp"

namespace server
{
    namespace scripting
    {
        Controller::Controller(Ref<Script> script) : script(script)
        {
            assert(script != nullptr);
        }
        Controller::~Controller()
        {
        }
    }
}