#pragma once
#include "net/transport/transport_layer.hpp"
#include "server_config.hpp"

#include <memory>

namespace valk {

    class Server {
    public:
        Server() = default;

        Server(const Server&) = delete;
        Server(Server&&)      = delete;

        // May throw any amount of errors
        void init();

    private:
        ServerConfig                          m_configuration{};
        std::unique_ptr<net::ITransportLayer> m_layer{};
    };

} // namespace valk
