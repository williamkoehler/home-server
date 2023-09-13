#pragma once
#include "library_information.hpp"
#include "common.hpp"
#include "interface/method.hpp"
#include "interface/property.hpp"
#include <scripting/script.hpp>
#include <scripting/value.hpp>
#include <scripting/interface/event.hpp>

namespace server
{
    namespace scripting
    {
        namespace native
        {
            class NativeScript;

            class Context
            {
              public:
                /// @brief Add attribute from json
                ///
                /// @param name Attribute name
                /// @param json Attribute value (in json format)
                /// @return Successfulness
                virtual bool AddAttribute(const std::string& name, const char* json) = 0;

                /// @brief Remove attribute
                ///
                /// @param name Attribute name
                /// @return true Attribute was successfuly removed
                /// @return false Attribute does not exist
                virtual bool RemoveAttribute(const std::string& name) = 0;

                /// @brief Clear attributes
                ///
                virtual void ClearAttributes() = 0;

                /// @brief Add property
                ///
                /// @param name Property name
                /// @return Successfulness
                virtual bool AddProperty(const std::string& name, UniqueRef<Property> property) = 0;

                /// @brief Remove property
                ///
                /// @param name Property name
                /// @return true Property was successfuly removed
                /// @return false Attribute does not exist
                virtual bool RemoveProperty(const std::string& name) = 0;

                /// @brief Clear properties
                ///
                virtual void ClearProperties() = 0;

                /// @brief Add method
                ///
                /// @param name Method name
                /// @param method Method definition
                /// @return Successfulness
                virtual bool AddMethod(const std::string& name, UniqueRef<Method> method) = 0;

                /// @brief Remove method
                ///
                /// @param name Method name
                /// @return true Method was successfully removed
                /// @return false Method does not exist
                virtual bool RemoveMethod(const std::string& name) = 0;

                /// @brief Clear methods
                ///
                virtual void ClearMethods() = 0;

                /// @brief Add event
                ///
                /// @param name Event name
                /// @return Event Event reference
                virtual Event AddEvent(const std::string& name) = 0;

                /// @brief Remove event
                ///
                /// @param name Event name
                /// @return true Event was successfully removed
                /// @return false Event does not exist
                virtual bool RemoveEvent(const std::string& name) = 0;

                /// @brief Clear events
                ///
                virtual void ClearEvents() = 0;
            };

            class NativeScriptImplementation : public boost::enable_shared_from_this<NativeScriptImplementation>
            {
              private:
                friend class NativeScript;

                Ref<View> view;

              public:
                NativeScriptImplementation()
                {
                }
                virtual ~NativeScriptImplementation()
                {
                }

                constexpr virtual ViewType GetViewType() const = 0;

                inline Ref<View> GetView() const
                {
                    return view;
                }

                /// @brief Initialize script
                ///
                /// @return Successfulness
                virtual bool Initialize(Context& context) = 0;
            };
        }
    }
}