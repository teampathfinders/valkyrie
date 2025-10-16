#include "connection_manager.hpp"
#include "event/event_provider.hpp"
#include "event/network_events.hpp" // This include is needed

namespace valk::net {
    ConnectionManager::ConnectionManager(std::unique_ptr<ITransportLayer>&& layer)
        : m_layer{std::forward<std::unique_ptr<ITransportLayer>>(layer)} {}

    void ConnectionManager::add_listeners() noexcept {
        event::EventProvider::add_listener<&ConnectionManager::connect_event>(this);
        event::EventProvider::add_listener<&ConnectionManager::disconnect_event>(this);
    }

    bool ConnectionManager::connect_event(const ConnectionEvent& connection) noexcept {
        const auto now = std::chrono::steady_clock::now();

        const PartialConnection new_connection{
            {connection.network_id}, static_cast<uint64_t>(now.time_since_epoch().count())
        };

        this->m_in_progress_connections[connection.network_id] = new_connection;

        return false;
    }

    bool ConnectionManager::disconnect_event(DisconnectionEvent& connection) noexcept {
        if (this->m_in_progress_connections.contains(connection.network_id)) {
            this->m_in_progress_connections.erase(connection.network_id);
        }

        return false;
    }
} // namespace valk::net