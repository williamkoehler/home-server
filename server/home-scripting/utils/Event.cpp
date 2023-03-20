#include "Event.hpp"
#include "../Script.hpp"

namespace server
{
    namespace scripting
    {
        Event::Event() : base(boost::make_shared<EventBase>())
        {
        }
        Event::~Event()
        {
        }

        EventConnection Event::Bind(Ref<View> view, const std::string& method)
        {
            Ref<EventEntry> entry = boost::make_shared<EventEntry>(EventEntry{
                .base = base,
                .view = std::move(view),
                .method = method,
            });

            base->push_back(entry);

            return EventConnection(std::move(entry));
        }

        void Event::Invoke(const Value& parameter) const
        {
            for (const Ref<EventEntry>& entry : *base)
            {
                Ref<View> view = entry->view.lock();
                if (view != nullptr)
                    view->Invoke(entry->method, parameter);
            }
        }

        EventConnection::EventConnection(Ref<EventEntry> entry) : entry(std::move(entry))
        {
        }
        EventConnection::EventConnection()
        {
        }
        EventConnection::EventConnection(EventConnection&& eventConnection) noexcept
            : entry(std::exchange(eventConnection.entry, WeakRef<EventEntry>()))
        {
        }
        EventConnection::~EventConnection()
        {
            Unbind();
        }

        void EventConnection::operator=(EventConnection&& other) noexcept
        {
            std::swap(entry, other.entry);
        }

        void EventConnection::Unbind()
        {
            Ref<EventEntry> r = entry.lock();
            if (r != nullptr)
            {
                // Find entry
                EventBase::iterator it = boost::range::find(*r->base, r);
                if (it != r->base->end())
                {
                    // Get last element
                    EventBase::iterator it2 = r->base->end() - 1;

                    // Swap with last element
                    std::iter_swap(it, it2);

                    // Pop last element
                    r->base->pop_back();
                }
            }

            entry.reset();
        }
    }
}