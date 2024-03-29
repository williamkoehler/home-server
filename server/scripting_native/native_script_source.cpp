#include "native_script_source.hpp"
#include "native_script.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScriptSource::NativeScriptSource(identifier_t id, const std::string& name, uint8_t flags,
                                                   sdk::CreateScriptCallback<> callback)
                : ScriptSource(id, name, std::string_view("", 0)), flags(flags), callback(callback)
            {
            }
            NativeScriptSource::~NativeScriptSource()
            {
            }
            Ref<NativeScriptSource> NativeScriptSource::Create(identifier_t id, const std::string& name, uint8_t flags,
                                                               sdk::CreateScriptCallback<> callback)
            {
                return boost::make_shared<NativeScriptSource>(id, name, flags, callback);
            }

            Ref<Script> NativeScriptSource::CreateScript(const Ref<sdk::View>& view)
            {
                assert(view != nullptr);

                try
                {
                    // Create native script
                    Ref<Script> script =
                        NativeScript::Create(view, boost::dynamic_pointer_cast<NativeScriptSource>(shared_from_this()));

                    if (script == nullptr)
                        LOG_ERROR("Native script '{0}' create callback returned an invalid script.", name);

                    return script;
                }
                catch (std::exception e)
                {
                    LOG_ERROR("Exception was thrown while creating new native script '{0}'.", name);
                    return nullptr;
                }
            }

            void NativeScriptSource::JsonGetConfig(rapidjson::Value& output,
                                                   rapidjson::Document::AllocatorType& allocator) const
            {
                (void)allocator;

                assert(output.IsObject());
            }
            bool NativeScriptSource::JsonSetConfig(const rapidjson::Value& input)
            {
                assert(input.IsObject());

                return false;
            }
        }
    }
}