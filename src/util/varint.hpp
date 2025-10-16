#pragma once

#include <concepts>
#include <cstdint>
#include <string>

namespace valk {
    template <std::integral T = uint32_t> struct VarInt {
        T value{};

        constexpr VarInt() = default;
        constexpr VarInt(T v) : value(v) {};

        constexpr operator T() const { return value; };

        constexpr VarInt& operator=(T v) {
            value = v;
            return *this;
        }

        constexpr VarInt& operator++() {
            ++value;
            return *this;
        }

        constexpr VarInt operator++(int) {
            auto tmp = *this;
            ++value;
            return tmp;
        }
    };

    template <typename T> constexpr std::make_unsigned_t<T> zigzag_encode(T v) {
        using U = std::make_unsigned_t<T>;
        if constexpr (std::is_signed_v<T>) {
            return (static_cast<U>(v) << 1) ^ static_cast<U>(v >> (sizeof(T) * 8 - 1));
        } else {
            return static_cast<U>(v);
        }
    }

    template <typename T> constexpr T zigzag_decode(std::make_unsigned_t<T> v) {
        if constexpr (std::is_signed_v<T>) {
            return static_cast<T>((v >> 1) ^ (~(v & 1) + 1));
        } else {
            return static_cast<T>(v);
        }
    }

    // Strings
    enum class StringLengthEncoding { VarInt, Fixed32 };

    class VarIntString {
        std::string value{};

    public:
        constexpr VarIntString() = default;
        constexpr VarIntString(const std::string& str) : value(str) {}
        constexpr VarIntString(std::string&& str) noexcept : value(std::move(str)) {}
        constexpr VarIntString(const char* str) : value(str) {}

        constexpr std::string*       operator->() { return &value; }
        constexpr const std::string* operator->() const { return &value; }

        constexpr operator std::string&() { return value; }
        constexpr operator const std::string&() const { return value; }
    };

    template <typename T> struct string_length_encoding;

    template <> struct string_length_encoding<std::string> {
        static constexpr auto value = StringLengthEncoding::Fixed32;
    };

    template <> struct string_length_encoding<VarIntString> {
        static constexpr auto value = StringLengthEncoding::VarInt;
    };
} // namespace valk