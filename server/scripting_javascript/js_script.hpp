#pragma once
#include "common.hpp"
#include <scripting/script.hpp>

extern "C"
{
    duk_ret_t duk_exec_timeout(void* udata);
}

void duk_log_dump(duk_context* context);

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

            class JSScript final : public Script
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

                /// @brief Script properties
                ///
                robin_hood::unordered_node_map<std::string, Value> propertyMap;

                /// @brief Script event names
                ///
                // boost::container::vector<std::string> eventList;

                void InitializeAttributes();
                void InitializeProperties();
                void InitializeMethods();
                void InitializeEvents();
                void InitializeControllers();

                static duk_ret_t duk_get_property(duk_context* context);
                static duk_ret_t duk_set_property(duk_context* context);

                static duk_ret_t duk_invoke_event(duk_context* context);

              public:
                JSScript(const Ref<View>& view, const Ref<JSScriptSource>& scriptSource);
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
                void PrepareTimeout(size_t maxTime = 100);

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

                virtual Value GetProperty(const std::string& name) override;
                virtual void SetProperty(const std::string& name, const Value& value) override;

                virtual bool Invoke(const std::string& name, const Value& parameter) override;

                virtual void JsonGetProperties(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator,
                                               PropertyFlags propertyFlags = kPropertyFlag_Visible) override;
                virtual PropertyFlags JsonSetProperties(const rapidjson::Value& input,
                                                        PropertyFlags propertyFlags = kPropertyFlag_All) override;
            };
        }
    }
}