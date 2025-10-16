#pragma once
#include <cstdint>
#include <string>

namespace valk {
    struct ServerConfig {
        bool        is_editor_mode  = false;
        std::string server_name     = "Dedicated Server";
        std::string server_version  = "1.0.0";
        uint16_t    max_connections = 50;
        uint16_t    port            = 19132;
        std::string level_name      = "Bedrock Level";
    };
} // namespace valk