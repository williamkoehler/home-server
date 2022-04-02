#include "Script.hpp"
#include "utils/Event.hpp"
#include "utils/Property.hpp"

namespace server
{
    namespace scripting
    {
        Script::Script(Ref<View> view, Ref<ScriptSource> scriptSource) : view(view), scriptSource(scriptSource)
        {
            assert(view != nullptr);
            assert(view->GetWorker() != nullptr);
            assert(scriptSource != nullptr);
        }
        Script::~Script()
        {
        }

        Ref<Property> Script::GetProperty(const std::string& id)
        {
            // Lock main mutex
            boost::lock_guard lock(mutex);

            const robin_hood::unordered_node_map<std::string, Ref<Property>>::const_iterator it = propertyList.find(id);
            if (it == propertyList.end())
                return nullptr;

            return it->second;
        }

        Ref<Event> Script::GetEvent(const std::string& id)
        {
            // Lock main mutex
            boost::lock_guard lock(mutex);

            const robin_hood::unordered_node_map<std::string, Ref<Event>>::const_iterator it = eventList.find(id);
            if (it == eventList.end())
                return nullptr;

            return it->second;
        }

        bool Script::PostInvoke(const std::string& event, Ref<EventCaller> caller)
        {
            GetWorker()->GetContext().dispatch(boost::bind(&Script::Invoke, shared_from_this(), event, caller));

            return true;
        }

        void Script::TakeSnapshot()
        {
            // Lock main and snapshot mutex
            boost::lock(mutex, snapshotMutex);
            boost::lock_guard lock2(mutex, boost::adopt_lock);
            boost::lock_guard lock(snapshotMutex, boost::adopt_lock);

            // Prepare rapidjson snapshot
            rapidjson::Document::AllocatorType& allocator = snapshot.GetAllocator();

            snapshot.SetNull();
            allocator.Clear();

            snapshot.SetObject();

            snapshot.MemberReserve(propertyList.size(), allocator);
            for (auto& [id, property] : propertyList)
            {
                // Add property
                snapshot.AddMember(rapidjson::Value(id.data(), id.size(), allocator), property->JsonGet(allocator),
                                   allocator);
            }
        }

        void Script::JsonGet(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            // Lock main mutex
            boost::lock_guard lock(mutex);

            // Reserve memory
            output.MemberReserve(attributeList.size(), allocator);
            for (auto& [id, attribute] : attributeList)
                output.AddMember(rapidjson::Value(id.data(), id.size(), allocator),
                                 rapidjson::Value(attribute, allocator, true), allocator);
        }
        void Script::JsonSet(rapidjson::Value& input)
        {
            throw std::exception();
        }

        void Script::JsonGetState(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            boost::lock_guard lock(snapshotMutex);

            output.CopyFrom(snapshot, allocator, true);
        }
        void Script::JsonSetState(rapidjson::Value& input)
        {
            LOG_CODE_MISSING("Script json set state");
        }
    }
}