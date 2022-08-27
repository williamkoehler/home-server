#include "JSScriptProvider.hpp"
#include "JSScriptSource.hpp"
#include "home-scripting-javascript/JSScriptSource.hpp"
#include "home-scripting/ScriptManager.hpp"

namespace server
{
    namespace scripting
    {
        namespace javascript
        {
            JSScriptProvider::JSScriptProvider()
            {
            }
            JSScriptProvider::~JSScriptProvider()
            {
            }
            Ref<ScriptProvider> JSScriptProvider::Create()
            {
                Ref<JSScriptProvider> provider = boost::make_shared<JSScriptProvider>();

                LOG_INFO("Initializing javascript script provider.");

                return provider;
            }

            Ref<ScriptSource> JSScriptProvider::CreateScriptSource(identifier_t id, const std::string& name,
                                                                   ScriptUsage usage, const std::string_view& data)
            {
                return JSScriptSource::Create(id, name, usage, data);
            }
        }
    }
}