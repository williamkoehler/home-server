#pragma once
#include "common.hpp"
#include "duktape.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/tools/Controller.hpp>

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

                struct ContextDeleter
                {
                    void operator()(void* context)
                    {
                        if (context != nullptr)
                            duk_destroy_heap((duk_context*)context);
                    }
                };

                std::unique_ptr<void, ContextDeleter> context;
                uint64_t checksum;

                /// @brief Script properties ordered by id
                ///
                boost::container::vector<Ref<Value>> propertyByIDList;

                /// @brief Script events ordered by id
                ///
                boost::container::vector<Ref<Event>> eventByIDList;

                /// @brief Script controllers
                ///
                robin_hood::unordered_node_map<duk_int_t, Ref<Controller>> controllerList;

                /// @brief Initialize script safely
                ///
                /// @param context Duktape context
                /// @param udata Userdata
                /// @return Sucessfulness
                static duk_ret_t InitializeSafe(duk_context* context, void* udata);

                void InitializeAttributes();
                void InitializeProperties();
                void InitializeMethods();
                void InitializeEvents();
                void InitializeControllers();

                bool Invoke(const std::string& method, Ref<Value> parameter);

                static duk_ret_t CreateTimer(duk_context* context);
                static duk_ret_t CreateInterval(duk_context* context);

                static duk_ret_t GetProperty(duk_context* context);
                static duk_ret_t SetProperty(duk_context* context);

                static duk_ret_t InvokeEvent(duk_context* context);

                static duk_ret_t TerminateSafe(duk_context* context, void* udata);

              public:
                JSScript(Ref<View> view, Ref<JSScriptSource> source);
                virtual ~JSScript();

                /// @brief Get duktape context
                ///
                /// @return Duktape context
                inline duk_context* GetDuktapeContext() const
                {
                    return (duk_context*)context.get();
                }

                /// @brief Initialize script
                ///
                /// @return Successful
                virtual bool Initialize() override;

                /// @brief Prepare script timeout
                ///
                /// @param maxTime Max execution time in milliseconds
                void PrepareTimeout(size_t maxTime = 5000);

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

                /// @brief Terminate script
                ///
                /// @return Successful
                virtual bool Terminate() override;
            };
        }
    }
}