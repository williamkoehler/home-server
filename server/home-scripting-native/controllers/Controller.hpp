#pragma once
#include "../common.hpp"

namespace server
{
    namespace scripting
    {

        namespace native
        {
            class NativeScriptImpl;

            enum class ControllerType
            {
                kUnknownController,
                kHttpController,
            };

            class Controller : public boost::enable_shared_from_this<Controller>
            {
              protected:
                WeakRef<NativeScriptImpl> script;

              public:
                Controller(const Ref<NativeScriptImpl>& script);
                virtual ~Controller();

                /// @brief Get controller type
                ///
                /// @return Controller type
                virtual ControllerType GetType() const
                {
                    return ControllerType::kUnknownController;
                };
            };
        }
    }
}