#pragma once
#include "common.hpp"
#include <home-scripting/Script.hpp>
#include <home-scripting/tools/Controller.hpp>

extern "C"
{
    duk_ret_t duk_exec_timeout(void* udata);
}

#define DUK_TEST_ENTER(context) duk_idx_t top1 = duk_get_top(context);
#define DUK_TEST_LEAVE(context, difference)                                                                            \
    duk_idx_t top2 = duk_get_top(context);                                                                             \
    assert((top2 - top1) == difference);

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

                /// @brief Script properties ordered by id
                ///
                boost::container::vector<Ref<Value>> propertyByIDList;

                /// @brief Script events ordered by id
                ///
                boost::container::vector<Ref<Event>> eventByIDList;

                /// @brief Script controllers
                ///
                robin_hood::unordered_node_map<duk_int_t, Ref<Controller>> controllerList;

                void InitializeAttributes();
                void InitializeProperties();
                void InitializeMethods();
                void InitializeEvents();
                void InitializeControllers();

                bool InvokeHandler(const std::string& method, Ref<Value> parameter);

                static duk_ret_t duk_get_property(duk_context* context);
                static duk_ret_t duk_set_property(duk_context* context);

                static duk_ret_t duk_invoke_event(duk_context* context);

              public:
                JSScript(Ref<View> view, Ref<JSScriptSource> scriptSource);
                virtual ~JSScript();

                /// @brief Get duktape context
                ///
                /// @return Duktape context
                inline duk_context* GetDuktapeContext() const
                {
                    return (duk_context*)context.get();
                }

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

                /// @brief Initialize script
                ///
                /// @return Successfulness
                virtual bool Initialize() override;
            };
        }
    }
}