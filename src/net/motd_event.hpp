#pragma once
#include "event/base_event.hpp"

#include <cstdint>
#include <string>

namespace valk {
    struct MOTDSetEvent : event::BaseEvent {
        const uint64_t server_guid{};

        std::string result{};
    };

    struct MOTDUpdateEvent : event::BaseEvent {
        const std::string value{};
    };
} // namespace valk