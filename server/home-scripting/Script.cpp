#include "Script.hpp"

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

        size_t Script::GetLazyUpdateInterval() const
        {
            const robin_hood::unordered_node_map<std::string, rapidjson::Document>::const_iterator it =
                attributeMap.find("lazy-update-interval");
            if (it != attributeMap.end() && (*it).second.IsUint64())
                return it->second.GetUint64();

            return 2; // Default 2 seconds
        }

        size_t Script::GetUpdateInterval() const
        {
            const robin_hood::unordered_node_map<std::string, rapidjson::Document>::const_iterator it =
                attributeMap.find("update-interval");
            if (it != attributeMap.end() && (*it).second.IsUint64())
                return it->second.GetUint64();

            return 10; // Default 10 seconds
        }

        bool Script::Initialize()
        {
            // Reset references
            attributeMap.clear();
            eventMap.clear();

            return true;
        }

        void Script::PostInvoke(const std::string& name, const Value& parameter)
        {
            Ref<Worker> worker = Worker::GetInstance();
            assert(worker != nullptr);

            worker->GetContext().dispatch(boost::bind(&Script::Invoke, shared_from_this(), name, parameter));
        }

        bool Script::LazyUpdate()
        {
            return Invoke("lazy-update", Value());
        }

        void Script::PostLazyUpdate()
        {
            Ref<Worker> worker = Worker::GetInstance();
            assert(worker != nullptr);

            worker->GetContext().dispatch(boost::bind(&Script::LazyUpdate, shared_from_this()));
        }

        bool Script::Update()
        {
            return Invoke("update", Value());
        }

        void Script::PostUpdate()
        {
            Ref<Worker> worker = Worker::GetInstance();
            assert(worker != nullptr);

            worker->GetContext().dispatch(boost::bind(&Script::Update, shared_from_this()));
        }

        EventConnection Script::Bind(const std::string& event, const Ref<View>& view, const std::string& method)
        {
            const robin_hood::unordered_node_map<std::string, Event>::iterator it = eventMap.find(event);
            if (it != eventMap.end())
                return it->second.Bind(view, method);

            return EventConnection();
        }

        void Script::JsonGetAttributes(rapidjson::Value& output, rapidjson::Document::AllocatorType& allocator)
        {
            assert(output.IsObject());

            output.MemberReserve(attributeMap.size(), allocator);

            for (auto& [id, attribute] : attributeMap)
            {
                output.AddMember(rapidjson::Value(id.data(), id.size(), allocator),
                                 rapidjson::Value(attribute, allocator, true), allocator);
            }
        }
    }
}