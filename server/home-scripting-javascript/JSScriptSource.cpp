#include "JSScriptSource.hpp"
#include "JSScript.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            JSScriptSource::JSScriptSource(identifier_t id, const std::string& name, ScriptUsage usage,
                                           const std::string_view& data)
                : ScriptSource(id, name, usage, data)
            {
            }
            JSScriptSource::~JSScriptSource()
            {
            }
            Ref<JSScriptSource> JSScriptSource::Create(identifier_t id, const std::string& name, ScriptUsage usage,
                                                       const std::string_view& data)
            {
                return boost::make_shared<JSScriptSource>(id, name, usage, data);
            }

            Ref<Script> JSScriptSource::CreateScript(Ref<View> view)
            {
                Ref<Script> script =
                    boost::make_shared<JSScript>(view, boost::dynamic_pointer_cast<JSScriptSource>(shared_from_this()));

                // Keep weak reference to allow content update
                scripts.push_back(script);

                return script;
            }
        }
    }
}