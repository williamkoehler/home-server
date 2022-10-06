#include "Script.hpp"

#include "tasks/TimerTask.hpp"

namespace server
{
    namespace scripting
    {
        Script::Script(Ref<View> view, Ref<ScriptSource> scriptSource) : view(view), scriptSource(scriptSource)
        {
            assert(view != nullptr);
            assert(scriptSource != nullptr);
        }
        Script::~Script()
        {
        }

        Ref<Value> Script::GetProperty(const std::string& name)
        {
            const robin_hood::unordered_node_map<std::string, Ref<Value>>::const_iterator it = propertyList.find(name);
            if (it == propertyList.end())
                return nullptr;

            return it->second;
        }

        Ref<Event> Script::GetEvent(const std::string& name)
        {
            const robin_hood::unordered_node_map<std::string, Ref<Event>>::const_iterator it = eventList.find(name);
            if (it == eventList.end())
                return nullptr;

            return it->second;
        }

        bool Script::Initialize()
        {
            // Reset references
            attributeList.clear();
            propertyList.clear();
            eventList.clear();

            // Clear tasks
            for (const WeakRef<Task>& task : taskList)
            {
                if (Ref<Task> r = task.lock())
                    r->Cancel();
            }
            taskList.clear();

            return true;
        }

        void Script::AddTimerTask(const std::string& method, size_t interval)
        {
            Ref<Task> task = TimerTask::Create(shared_from_this(), method, interval);
            if (task != nullptr)
                taskList.push_back(task);
        }

        void Script::CleanTasks()
        {
            taskList.erase(std::remove_if(taskList.begin(), taskList.end(),
                                          [](const boost::weak_ptr<Task>& task) -> bool const
                                          { return task.expired(); }),
                           taskList.end());
        }

        void Script::PostInvoke(const std::string& name, Ref<Value> parameter)
        {
            Ref<Worker> worker = Worker::GetInstance();
            assert(worker != nullptr);

            worker->GetContext().dispatch(boost::bind(&Script::Invoke, shared_from_this(), name, parameter));
        }

        void Script::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Build script attributes
            rapidjson::Value attributesJson = rapidjson::Value(rapidjson::kObjectType);
            attributesJson.MemberReserve(attributeList.size(), allocator);

            for (auto& [id, attribute] : attributeList)
                attributesJson.AddMember(rapidjson::Value(id.data(), id.size(), allocator),
                                         rapidjson::Value(attribute, allocator, true), allocator);

            output.AddMember("attributes", attributesJson, allocator);
        }
        void Script::JsonSet(rapidjson::Value& input)
        {
            throw std::exception();
        }

        void Script::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.MemberReserve(propertyList.size(), allocator);
            for (auto& [id, property] : propertyList)
            {
                // Add property
                output.AddMember(rapidjson::Value(id.data(), id.size(), allocator), property->JsonGet(allocator),
                                 allocator);
            }
        }
        void Script::JsonSetState(rapidjson::Value& input)
        {
            assert(input.IsObject());

            for (rapidjson::Value::MemberIterator propertyIt = input.MemberBegin(); propertyIt != input.MemberEnd();
                 propertyIt++)
            {
                Ref<Value> property =
                    GetProperty(std::string(propertyIt->name.GetString(), propertyIt->name.GetStringLength()));

                if (property != nullptr)
                    property->JsonSet(propertyIt->value);
            }
        }
    }
}