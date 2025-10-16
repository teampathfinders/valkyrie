#pragma once
#include "RakNet/RakNetTypes.h"
#include "net/transport/transport_layer.hpp"

namespace valk {
    struct MOTDUpdateEvent;
}
namespace valk::rak {

    class RakTransportLayer : public net::ITransportLayer {
    public:
        void initialize(const net::NetConfig& config) override;
        void listen(std::atomic_bool& alive) override;

    private:
        void set_motd(MOTDUpdateEvent& event);

    private:
        RakNet::RakPeerInterface* m_rak_peer{nullptr};
        RakNet::SocketDescriptor  m_descriptor{};
        RakNet::RakNetGUID        m_guid{RakNet::UNASSIGNED_RAKNET_GUID};
    };

} // namespace valk::rak
