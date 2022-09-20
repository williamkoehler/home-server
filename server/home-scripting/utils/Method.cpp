#include "Method.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Method::Method(const std::string& name, Ref<Script> script, MethodCallback<> callback)
            : name(name), script(script), callback(callback)
        {
            assert(callback != nullptr);
        }
        Method::~Method()
        {
        }

        Ref<Method> Method::Create(const std::string& name, Ref<Script> script, MethodCallback<> callback)
        {
            return boost::make_shared<Method>(name, script, callback);
        }

        bool Method::Invoke(Ref<Value> parameter)
        {
            Ref<Script> r = script.lock();

            if (r != nullptr)
                return (r.get()->*callback)(name, parameter);

            return false;
        }

        void Method::PostInvoke(Ref<Value> parameter)
        {
            Ref<Worker> worker = Worker::GetInstance();
            assert(worker != nullptr);

            worker->GetContext().dispatch(boost::bind(&Method::Invoke, shared_from_this(), parameter));
        }
    }
}