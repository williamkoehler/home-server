#include "NativeScriptSource.hpp"
#include "NativeScript.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScriptSource::NativeScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                                                   CreateScriptCallback<> callback)
                : ScriptSource(id, name, usage, std::string_view("", 0)), callback(callback)
            {
            }
            NativeScriptSource::~NativeScriptSource()
            {
            }
            Ref<NativeScriptSource> NativeScriptSource::Create(identifier_t id, const std::string& name,
                                                               ScriptUsage usage, CreateScriptCallback<> callback)
            {
                return boost::make_shared<NativeScriptSource>(id, name, usage, callback);
            }

            Ref<Script> NativeScriptSource::CreateScript(const Ref<View>& view)
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
        }
    }
}