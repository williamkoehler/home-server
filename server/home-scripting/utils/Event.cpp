#include "Event.hpp"
#include "../Script.hpp"
#include "Method.hpp"

namespace server
{
    namespace scripting
    {
        Event::Event()
        {
        }
        Event::~Event()
        {
        }

        Ref<Event> Event::Create()
        {
            return boost::make_shared<Event>();
        }

        void Event::Add(Ref<Script> script, const std::string& method)
        {
            assert(script != nullptr);

            Ref<Method> r = script->GetMethod(method);

            if (r != nullptr)
                methodList.push_back(r);
        }

        void Event::Invoke(Ref<Value> parameter)
        {
            for (boost::container::vector<WeakRef<Method>>::iterator it = methodList.begin(); it != methodList.end();
                 it++)
            {
                Ref<Method> r = it->lock();

                if (r != nullptr)
                    r->Invoke(parameter);
                else
                    it = methodList.erase(it);
            }
        }

        void Event::PostInvoke(Ref<Value> parameter)
        {
            for (boost::container::vector<WeakRef<Method>>::iterator it = methodList.begin(); it != methodList.end();
                 it++)
            {
                Ref<Method> r = it->lock();

                if (r != nullptr)
                    r->PostInvoke(parameter);
                else
                    it = methodList.erase(it);
            }
        }

        void Event::Remove(Ref<Script> script, const std::string& method)
        {
            assert(script != nullptr);

            Ref<Method> r1 = script->GetMethod(method);

            for (boost::container::vector<WeakRef<Method>>::iterator it = methodList.begin(); it != methodList.end();
                 it++)
            {
                Ref<Method> r2 = it->lock();

                if (r2 == r1 || r2 == nullptr)
                    it = methodList.erase(it);
            }
        }
    }
}