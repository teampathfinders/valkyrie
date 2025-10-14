#include "event_provider.hpp"

namespace valk::event {
    void EventProvider::deafen(const ListenerId id) {
        auto& self = instance();

        std::shared_lock lock{event_list_mutex};

        auto& [listeners]                                      = self.events[id.event];
        listeners[static_cast<size_t>(id.listener_index)].deaf = true;
    }

    void EventProvider::undeafen(ListenerId id) {
        auto& self = instance();

        std::shared_lock lock{event_list_mutex};

        auto& [listeners]                                      = self.events[id.event];
        listeners[static_cast<size_t>(id.listener_index)].deaf = false;
    }

    EventProvider::ListenerId EventProvider::push_listener(
        std::unique_ptr<IListenerBase>&& listener, const EventId event_id
    ) {
        auto& [listeners] = this->events[event_id];
        std::unique_lock lock{event_list_mutex};
        const auto       id = ListenerId{
                  .event = event_id, .listener_index = static_cast<uint32_t>(listeners.size())
        };

        listeners.emplace_back(false, std::move(listener));
        return id;
    }
} // namespace valk::event