#pragma once
#include "event/base_event.hpp"
#include "net/transport/connection.hpp"
#include "util/binary_span.hpp"

namespace valk::net {

    struct RawGamePacketRecv : event::CancelableEvent {
        const NetworkIdentifier network_id;
        NonOwnedBinarySpan      packet_data{};
    };

    struct ConnectionEvent : event::CancelableEvent {
        const NetworkIdentifier network_id;
    };

    struct DisconnectionEvent : event::CancelableEvent {
        const NetworkIdentifier network_id;
    };
} // namespace valk::net