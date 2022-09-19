#include "NativeScriptSource.hpp"

namespace server
{
    namespace scripting
    {
        namespace native
        {
            NativeScriptSource::NativeScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                                                   CreateScriptCallback* callback)
                : ScriptSource(id, name, usage, std::string_view("", 0)), callback(callback)
            {
            }
            NativeScriptSource::~NativeScriptSource()
            {
            }
            Ref<NativeScriptSource> NativeScriptSource::Create(identifier_t id, const std::string& name,
                                                               ScriptUsage usage, CreateScriptCallback* callback)
            {
                return boost::make_shared<NativeScriptSource>(id, name, usage, callback);
            }

            Ref<Script> NativeScriptSource::CreateScript(Ref<View> view)
            {
                assert(view != nullptr);

                try
                {
                    // Create native script
                    Ref<NativeScript> result = nullptr;
                    callback(view, boost::dynamic_pointer_cast<NativeScriptSource>(shared_from_this()), &result);

                    return result;
                }
                catch (std::exception e)
                {
                    LOG_ERROR("Failed to create script from native source.");
                }

                return nullptr;
            }
        }
    }
}