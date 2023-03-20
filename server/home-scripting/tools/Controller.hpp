#pragma once
#include "../common.hpp"

namespace server
{
    namespace scripting
    {
        class Script;

        enum class ControllerType
        {
            kUnknownController,
            kDelayController,
            kTimerController,
            kHttpController,
        };

        class Controller : public boost::enable_shared_from_this<Controller>
        {
          protected:
            WeakRef<Script> script;

          public:
            Controller(const Ref<Script>& script);
            virtual ~Controller();

            /// @brief Get controller type
            ///
            /// @return Controller type
            virtual ControllerType GetType() const
            {
                return ControllerType::kUnknownController;
            };
        };

        template <class T = Script>
        using CallbackMethod = bool (T::*)(Ref<Controller> controller);

        template <class T>
        union CallbackMethodConversion
        {
            CallbackMethod<T> f1;
            CallbackMethod<> f2;
        };
    }
}