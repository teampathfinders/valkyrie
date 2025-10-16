#pragma once
#include <chrono>
#include <cstdint>

namespace valk::net {

    struct NetConfig {
        uint16_t                  port{19132};
        uint16_t                  max_connections{50};
        const char*               host{nullptr};
        std::chrono::milliseconds timeout{5000};
    };

} // namespace valk::net
