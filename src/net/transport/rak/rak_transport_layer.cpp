#include "rak_transport_layer.hpp"

#include "RakPeerInterface.h"
#include "event/event_provider.hpp"
#include "net/motd_event.hpp"
#include "net/net_config.hpp"
#include "net/transport/connection.hpp"
#include "util/binary_span.hpp"

#include <utility>

namespace valk::rak {
    void RakTransportLayer::initialize(const net::NetConfig& config) {
        this->m_rak_peer = RakNet::RakPeerInterface::GetInstance();
        this->m_guid     = m_rak_peer->GetMyGUID();

        this->m_rak_peer->SetMaximumIncomingConnections(config.max_connections);
        this->m_descriptor = RakNet::SocketDescriptor{config.port, config.host};

        const auto state =
            this->m_rak_peer->Startup(config.max_connections, &this->m_descriptor, 1);

        if (state != RakNet::RAKNET_STARTED) {
            throw std::runtime_error(
                std::format("Failed to start raknet, error: {}", std::to_underlying(state))
            );
        }
        event::EventProvider::add_listener<&RakTransportLayer::set_motd>(this);

        MOTDSetEvent event{.server_guid = this->m_guid.g};

        event::EventProvider::fire_event(event);

        this->m_rak_peer->SetOfflinePingResponse(event.result.c_str(), event.result.size());
    }

    void RakTransportLayer::listen(std::atomic_bool& alive) {

        while (alive) {
            event::EventProvider::fire_queued_events<net::UpdateMOTDQueueId>();

            const RakNet::Packet* packet{nullptr};

            while ((packet = this->m_rak_peer->Receive()) != nullptr) {
                NonOwnedBinarySpan packet_data{std::span{packet->data, packet->length}};
                const net::NetworkIdentifier identifier{packet->guid.g};
            }
        }
    }

    void RakTransportLayer::set_motd(MOTDUpdateEvent& event) {
        this->m_rak_peer->SetOfflinePingResponse(event.value.c_str(), event.value.size());
    }
} // namespace valk::rak