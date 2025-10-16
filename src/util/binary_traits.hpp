#pragma once
#include "error.hpp"

#include <bit>
#include <expected>

namespace valk {
    namespace detail {
        template <bool> class BinarySpan;
    } // namespace detail

    template <typename UserError> struct BinaryStreamError;
    template <typename T, std::endian endianness = std::endian::little>
    struct BinarySerializable;

    template <typename T, std::endian endianness>
    concept binary_serializeable = requires(const T& t, detail::BinarySpan<true>& b) {
        typename BinarySerializable<T, endianness>::error_type;
        {
            BinarySerializable<T, endianness>::serialize(t, b)
        } -> std::same_as<std::expected<
            void, BinaryStreamError<typename BinarySerializable<T, endianness>::error_type>>>;
    };

    template <typename T, std::endian endianness> struct binary_serializable_t {
        constexpr static bool value = binary_serializeable<T, endianness>;
        using type                  = T;
    };

    template <typename T, std::endian endianness> struct BinaryDeserializable;

    template <typename T, std::endian endianness, bool supports_resize = false>
    concept binary_deserializable = requires(detail::BinarySpan<false>& u) {
        typename BinaryDeserializable<T, endianness>::error_type;
        {
            BinaryDeserializable<T, endianness>::deserialize(u)
        } -> std::same_as<std::expected<
            T, BinaryStreamError<typename BinaryDeserializable<T, endianness>::error_type>>>;
    };
} // namespace valk