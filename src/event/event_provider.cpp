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
} // namespace valk::event