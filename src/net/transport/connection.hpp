#pragma once
#include <cstdint>
#include <functional>

namespace valk::net {

    struct NetworkIdentifier {
        uint64_t guid{0};

        constexpr bool operator==(const NetworkIdentifier& other) const noexcept {
            return guid == other.guid;
        }
    };

    struct Connection {
        NetworkIdentifier identifier{};
    };

} // namespace valk::net

template <> struct std::hash<valk::net::NetworkIdentifier> {
    constexpr std::size_t operator()(const valk::net::NetworkIdentifier& id) const noexcept {
        return std::hash<uint64_t>()(id.guid);
    }
}; // namespace std