#include "js_script_source.hpp"
#include "js_script.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            JSScriptSource::JSScriptSource(identifier_t id, const std::string& name, const std::string_view& content)
                : ScriptSource(id, name, content)
            {
            }
            JSScriptSource::~JSScriptSource()
            {
            }
            Ref<JSScriptSource> JSScriptSource::Create(identifier_t id, const std::string& name,
                                                       const std::string_view& content)
            {
                return boost::make_shared<JSScriptSource>(id, name, content);
            }

            void JSScriptSource::SetContent(const std::string_view& v)
            {
                ScriptSource::SetContent(v);

                // Re-Initialze scripts
                for (const WeakRef<JSScript>& script : scriptList)
                {
                    if (Ref<JSScript> r = script.lock())
                        r->Initialize(); // Re-Initialize
                }
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

            void JSScriptSource::JsonGetConfig(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator) const
            {
                (void)allocator;
                
                assert(output.IsObject());
            }
            bool JSScriptSource::JsonSetConfig(const rapidjson::Value& input)
            {
                assert(input.IsObject());

                return true;
            }
        }
    }
}