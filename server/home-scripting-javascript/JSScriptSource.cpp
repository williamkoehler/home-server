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

            bool JSScriptSource::SetContent(const std::string_view& v)
            {
                if (ScriptSource::SetContent(v))
                {
                    // Re-Initialze scripts
                    for (const WeakRef<JSScript>& script : scriptList)
                    {
                        if (Ref<JSScript> r = script.lock())
                            r->Initialize(); // Re-Initialize
                    }

                    return true;
                }
                else
                    return false;
            }

            Ref<Script> JSScriptSource::CreateScript(const Ref<View>& view)
            {
                Ref<JSScript> script =
                    boost::make_shared<JSScript>(view, boost::dynamic_pointer_cast<JSScriptSource>(shared_from_this()));

                if (script != nullptr)
                {
                    // Keep weak reference to scripts
                    scriptList.push_back(script);
                }

                return script;
            }

            void JSScriptSource::CleanScripts()
            {
                scriptList.erase(std::remove_if(scriptList.begin(), scriptList.end(),
                                                [](const boost::weak_ptr<JSScript>& script) -> bool
                                                { return script.expired(); }),
                                 scriptList.end());
            }
        }
    }
}