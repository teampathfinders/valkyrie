#include "event_provider.hpp"

#include "spdlog/spdlog.h"

namespace valk::event {
    void EventProvider::deafen(const ListenerId id) {
        auto& self = instance();

        std::shared_lock lock{MUTEX};

        auto& [listeners]                                      = self.m_events[id.event];
        listeners[static_cast<size_t>(id.listener_index)].deaf = true;
    }

    void EventProvider::undeafen(const ListenerId id) {
        auto& self = instance();

        std::shared_lock lock{MUTEX};

        auto& [listeners]                                      = self.m_events[id.event];
        listeners[static_cast<size_t>(id.listener_index)].deaf = false;
    }

    void EventProvider::fire_queued_events(const EventQueueId queue_id) {
        auto& self  = instance();
        auto& queue = self.m_queues[queue_id];

        std::unique_ptr<IQueuedEvent> event{};
        while (queue.try_dequeue(event)) {
            event->fire();
        }
    }

    EventProvider::ListenerId EventProvider::push_listener(
        std::unique_ptr<IListenerBase>&& listener, const EventId event_id
    ) {
        auto& [listeners] = this->m_events[event_id];
        std::unique_lock lock{MUTEX};
        const auto       id = ListenerId{
                  .event = event_id, .listener_index = static_cast<uint32_t>(listeners.size())
        };

        listeners.emplace_back(false, std::move(listener));
        return id;
    }

    void EventProvider::queue_event_internal(
        const EventQueueId queue_id, std::unique_ptr<IQueuedEvent>&& event
    ) {
        this->m_queues[queue_id].enqueue(std::move(event));
    }
} // namespace valk::event