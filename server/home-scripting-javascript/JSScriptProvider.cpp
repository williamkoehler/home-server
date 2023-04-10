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

            boost::container::vector<StaticScriptSource> JSScriptProvider::GetStaticScriptSources()
            {
                // The javascript provider does not load any scripts
                return boost::container::vector<StaticScriptSource>();
            }

            Ref<ScriptSource> JSScriptProvider::CreateScriptSource(identifier_t id, const std::string& name,
                                                                   const std::string_view& data)
            {
                return JSScriptSource::Create(id, name, data);
            }
        }
    }
}