#pragma once
#include "transport/connection.hpp"
#include "transport/transport_layer.hpp"

#include <chrono>
#include <memory>

namespace valk::net {
    struct DisconnectionEvent;
}
namespace valk::net {
    struct ConnectionEvent;

    struct PartialConnection : Connection {
        uint64_t initialisation_point{};
    };

    class ConnectionManager {
    public:
        explicit ConnectionManager(std::unique_ptr<ITransportLayer>&& layer);
        ConnectionManager(ConnectionManager&&)                 = delete;
        ConnectionManager(const ConnectionManager&)            = delete;
        ConnectionManager& operator=(ConnectionManager&&)      = delete;
        ConnectionManager& operator=(const ConnectionManager&) = delete;

    private:
        void add_listeners() noexcept;

        bool connect_event(const ConnectionEvent& connection) noexcept;
        bool disconnect_event(DisconnectionEvent& connection) noexcept;

    private:
        std::unique_ptr<ITransportLayer>                         m_layer{};
        std::unordered_map<NetworkIdentifier, PartialConnection> m_in_progress_connections{};
    };

} // namespace valk::net
