#include "server.hpp"

#include "net/transport/rak/rak_transport_layer.hpp"

namespace valk {
    void Server::init() { this->m_layer = std::make_unique<rak::RakTransportLayer>(); }
} // namespace valk