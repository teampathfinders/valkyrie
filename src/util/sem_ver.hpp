#pragma once
#include <cstdint>
#include <functional>

namespace valk {

    class SemVer {
    public:
        constexpr SemVer(
            const uint8_t major, const uint8_t minor, const uint8_t patch,
            const uint8_t revision
        )
            : m_major(major), m_minor(minor), m_patch(patch), m_revision(revision) {}
        constexpr SemVer() = default;

        constexpr uint8_t major() const noexcept { return m_major; }
        constexpr uint8_t minor() const noexcept { return m_minor; }
        constexpr uint8_t patch() const noexcept { return m_patch; }
        constexpr uint8_t revision() const noexcept { return m_revision; }

        constexpr uint32_t combined() const noexcept {
            uint32_t combined  = 0;
            combined           = static_cast<uint32_t>(this->major()) << 24;
            combined          |= static_cast<uint32_t>(this->minor() << 16);
            combined          |= static_cast<uint32_t>(this->patch()) << 8;
            combined          |= static_cast<uint32_t>(this->revision());
            return combined;
        }

        constexpr operator uint32_t() const noexcept { return combined(); }

        constexpr bool operator==(const SemVer& other) const noexcept {
            return (this->combined() == other.combined());
        }

        constexpr bool operator!=(const SemVer& other) const noexcept {
            return (this->combined() != other.combined());
        }

        constexpr bool operator<(const SemVer& other) const noexcept {
            return (this->combined() < other.combined());
        }

        constexpr bool operator>(const SemVer& other) const noexcept {
            return (this->combined() > other.combined());
        }

        constexpr bool operator<=(const SemVer& other) const noexcept {
            return (this->combined() <= other.combined());
        }

        constexpr bool operator>=(const SemVer& other) const noexcept {
            return (this->combined() >= other.combined());
        }

    private:
        uint8_t m_major{};
        uint8_t m_minor{};
        uint8_t m_patch{};
        uint8_t m_revision{};
    };

} // namespace valk

template <> struct std::hash<valk::SemVer> {
    constexpr size_t operator()(const valk::SemVer& ver) const noexcept {
        return ver.combined();
    }
}; // namespace std