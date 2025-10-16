#pragma once
#include <atomic>

namespace valk::net {
    struct NetConfig;

    struct UpdateMOTDQueueId {};

    class ITransportLayer {
    public:
        ITransportLayer()                                  = default;
        virtual ~ITransportLayer()                         = default;
        ITransportLayer(const ITransportLayer&)            = delete;
        ITransportLayer(ITransportLayer&&) noexcept        = delete;
        ITransportLayer& operator=(ITransportLayer&&)      = delete;
        ITransportLayer& operator=(const ITransportLayer&) = delete;

        // Can throw depending on the transport layer
        virtual void initialize(const NetConfig& config) = 0;
        // Can throw depending on the transport layer
        virtual void listen(std::atomic_bool& alive) = 0;
    };

} // namespace valk::net
