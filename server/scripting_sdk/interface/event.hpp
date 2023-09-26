#pragma once
#include "../common.hpp"
#include "../view/view.hpp"

namespace server
{
    namespace scripting
    {
        namespace sdk
        {
            class Value;

            class Event;
            struct EventEntry;

            typedef boost::container::vector<Ref<EventEntry>> EventBase;

            struct EventEntry final
            {
                Ref<EventBase> base;
                WeakRef<View> view;
                std::string method;
            };

            class EventConnection final
            {
              private:
                friend class Event;

                WeakRef<EventEntry> entry;

                EventConnection(const Ref<EventEntry>& entry) : entry(entry)
                {
                }

              public:
                EventConnection()
                {
                }
                EventConnection(const EventConnection& other) = delete;
                EventConnection(EventConnection&& other) noexcept
                    : entry(std::exchange(other.entry, WeakRef<EventEntry>()))
                {
                }
                ~EventConnection()
                {
                    Unbind();
                }

                void operator=(EventConnection&& other) noexcept
                {
                    std::swap(entry, other.entry);
                }

                void Unbind()
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
            };

            class Event final
            {
              protected:
                Ref<EventBase> base;

              public:
                Event() : base(boost::make_shared<EventBase>())
                {
                }
                ~Event()
                {
                }

                /// @brief Bind script method to event
                ///
                /// @param view Invokable view
                /// @param method Method name
                /// @return EventConnection Event connection
                EventConnection Bind(const Ref<View>& view, const std::string& method)
                {
                    Ref<EventEntry> entry = boost::make_shared<EventEntry>(EventEntry{
                        .base = base,
                        .view = view,
                        .method = method,
                    });

                    base->push_back(entry);

                    return EventConnection(entry);
                }

                /// @brief Invoke event
                ///
                /// @param parameter Parameter
                void Invoke(const Value& parameter) const
                {
                    for (const Ref<EventEntry>& entry : *base)
                    {
                        Ref<View> view = entry->view.lock();
                        if (view != nullptr)
                            view->Invoke(entry->method, parameter);
                    }
                }
            };
        }
    }
}