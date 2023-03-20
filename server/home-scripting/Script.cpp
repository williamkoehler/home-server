#include "Script.hpp"

#include "tasks/TimerTask.hpp"

namespace server
{
    namespace scripting
    {
        Script::Script(const Ref<View>& view, const Ref<ScriptSource>& scriptSource)
            : view(view), scriptSource(scriptSource)
        {
            assert(view != nullptr);
            assert(scriptSource != nullptr);
        }
        Script::~Script()
        {
        }

        bool Script::Initialize()
        {
            // Reset references
            attributeMap.clear();
            eventMap.clear();

            // Clear tasks
            for (const WeakRef<Task>& task : taskMap)
            {
                if (Ref<Task> r = task.lock())
                    r->Cancel();
            }
            taskMap.clear();

            return true;
        }

        void Script::AddTimerTask(const std::string& method, size_t interval)
        {
            Ref<Task> task = TimerTask::Create(shared_from_this(), method, interval);
            if (task != nullptr)
                taskMap.push_back(task);
        }

        void Script::CleanTasks()
        {
            taskMap.erase(std::remove_if(taskMap.begin(), taskMap.end(),
                                         [](const boost::weak_ptr<Task>& task) -> bool { return task.expired(); }),
                          taskMap.end());
        }

        void Script::PostInvoke(const std::string& name, const Value& parameter)
        {
            Ref<Worker> worker = Worker::GetInstance();
            assert(worker != nullptr);

            worker->GetContext().dispatch(boost::bind(&Script::Invoke, shared_from_this(), name, parameter));
        }

        EventConnection Script::Bind(const std::string& event, const Ref<View>& view, const std::string& method)
        {
            const robin_hood::unordered_node_map<std::string, Event>::iterator it = eventMap.find(event);
            if (it != eventMap.end())
                return it->second.Bind(view, method);

            return EventConnection();
        }

        void Script::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Build script attributes
            rapidjson::Value attributesJson = rapidjson::Value(rapidjson::kObjectType);
            attributesJson.MemberReserve(attributeMap.size(), allocator);

            for (auto& [id, attribute] : attributeMap)
                attributesJson.AddMember(rapidjson::Value(id.data(), id.size(), allocator),
                                         rapidjson::Value(attribute, allocator, true), allocator);

            output.AddMember("attributes", attributesJson, allocator);
        }
        void Script::JsonSet(rapidjson::Value& input)
        {
            (void)input;

            assert(false);
        }
    }
}