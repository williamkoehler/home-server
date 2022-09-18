#include "Method.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Method::Method(const std::string& name, MethodCallback<> callback) : name(name), callback(callback)
        {
            assert(callback != nullptr);
        }
        Method::~Method()
        {
        }

        Ref<Method> Method::Create(const std::string& name, MethodCallback<> callback)
        {
            return boost::make_shared<Method>(name, callback);
        }
    }
}