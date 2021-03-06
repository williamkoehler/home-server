#pragma once
#include "common.hpp"
#include "duktape.h"
#include <home-scripting/Script.hpp>

extern "C"
{
    duk_ret_t duk_exec_timeout(void* udata);
}

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            class JSScriptSource;

            class JSScript : public Script
            {
              private:
                size_t startTime;
                size_t maxTime;
                Ref<duk_context> context;
                uint64_t checksum;

                boost::container::vector<Ref<Property>> propertyByIDList;

                // Prepare script timeout
                void PrepareTimeout(size_t maxTime = 5);

                /// @brief Initialize script safely
                ///
                /// @param context Duktape context
                /// @param udata Userdata
                /// @return Sucessfulness
                static duk_ret_t InitializeSafe(duk_context* context, void* udata);

                void InitializeAttributes();
                void InitializeProperties();
                void InitializeEvents();

                /// @brief Invoke event safely
                ///
                /// @param context Duktape context
                /// @param udata Userdata
                /// @return Successfulness
                static duk_ret_t InvokeSafe(duk_context* context, void* udata);

                static duk_ret_t PropertyGetter(duk_context* context);
                static duk_ret_t PropertySetter(duk_context* context);

                static duk_ret_t TerminateSafe(duk_context* context, void* udata);

              public:
                JSScript(Ref<View> view, Ref<JSScriptSource> source);
                virtual ~JSScript();

                /// @brief Get duktape context
                ///
                /// @return Duktape context
                inline Ref<duk_context> GetDuktape()
                {
                    return context;
                }

                /// @brief Initialize script
                ///
                /// @return Successful
                virtual bool Initialize() override;

                /// @brief Terminate script
                ///
                /// @return Successful
                virtual bool Terminate() override;

                /// @brief Check if time has passed
                ///
                /// @return Successfulness
                inline bool CheckTimeout()
                {
                    // Test elapsed time
                    size_t currentTime = clock() / (CLOCKS_PER_SEC / 1000);
                    size_t elapsedTime = currentTime - startTime;

                    return elapsedTime > maxTime;
                }
            };
        }
    }
}