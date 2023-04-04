#include "Controller.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            Controller::Controller(const Ref<NativeScriptImpl>& script) : script(script)
            {
                assert(script != nullptr);
            }
            Controller::~Controller()
            {
            }
        }
    }
}