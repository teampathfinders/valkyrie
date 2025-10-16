#pragma once

#include "binary_traits.hpp"
#include "varint.hpp"

#include <cstdint>
#include <expected>
#include <fstream>
#include <optional>
#include <ranges>
#include <vector>

namespace valk {
    template <typename UserError> struct BinaryStreamError final : std::runtime_error {
        enum class Error { OutOfSpace, Malformed, User };
        Error                    kind{};
        std::optional<UserError> error{};

        explicit constexpr BinaryStreamError(
            const std::string& message, const Error error, std::optional<UserError>&& val
        )
            : std::runtime_error(message), kind(error), error(std::move(val)) {}

        static constexpr BinaryStreamError out_of_space() {
            return BinaryStreamError{
                "No space in stream for decoding", Error::OutOfSpace, std::nullopt
            };
        }

        static constexpr BinaryStreamError malformed() {
            return BinaryStreamError{
                "Malformed value in stream", Error::Malformed, std::nullopt
            };
        }

        static constexpr BinaryStreamError user(UserError&& error) {
            return BinaryStreamError{"User Defined Error", Error::User, std::move(error)};
        }
    };

    namespace detail {

        template <auto> class BinarySpanStorage;

        template <> class BinarySpanStorage<false> {
        protected:
            std::span<const uint8_t> backing_type{};
            size_t                   index{0};

        public:
            using BackingType                              = std::span<const uint8_t>;
            [[maybe_unused]] constexpr BinarySpanStorage() = default;
            explicit constexpr BinarySpanStorage(const BackingType& backing_type)
                : backing_type(backing_type) {}
            explicit constexpr BinarySpanStorage(BackingType&& backing_type)
                : backing_type(backing_type) {}
            constexpr void skip_n(size_t count = 1);

        private:
            constexpr BinarySpan<false>& cast();
        };

        template <> class BinarySpanStorage<true> {
        protected:
            std::vector<uint8_t> backing_type{};
            size_t               index{0};

        public:
            using BackingType             = std::vector<uint8_t>;
            constexpr BinarySpanStorage() = default;
            explicit constexpr BinarySpanStorage(const BackingType& backing_type)
                : backing_type(backing_type) {}
            explicit constexpr BinarySpanStorage(BackingType&& backing_type)
                : backing_type(std::move(backing_type)) {}

            template <typename T, std::endian e = std::endian::little>
                requires binary_serializeable<T, e>
            constexpr std::expected<
                void, BinaryStreamError<typename BinarySerializable<T, e>::error_type>>
            serialize(const T& t) {
                return BinarySerializable<T, e>::serialize(t, this->cast());
            }

            constexpr void skip_n(size_t count = 1);
            constexpr void write_byte(uint8_t byte);

            [[nodiscard]] constexpr BinarySpan<false> to_static_size() const;

        private:
            constexpr BinarySpan<true>& cast();
        };

        template <bool supports_resize>
        class BinarySpan : public BinarySpanStorage<supports_resize> {
        public:
            using storage     = BinarySpanStorage<supports_resize>;
            using BackingType = storage::BackingType;

            constexpr BinarySpan() = default;
            constexpr explicit BinarySpan(const BackingType& type)
                : storage(std::forward<const BackingType&>(type)) {}
            constexpr explicit BinarySpan(BackingType&& type) : storage(std::move(type)) {}

            [[nodiscard]] constexpr size_t size() const { return this->backing_type.size(); }
            [[nodiscard]] constexpr size_t remaining() const {

                if (this->size() == 0) {
                    return 0;
                }

                return this->size() - this->index;
            }

            void write_to(std::ofstream& stream) const {
                stream.write(
                    std::bit_cast<const char*>(this->backing_type.data()),
                    this->backing_type.size()
                );
            }

            constexpr void go_to(const size_t index) {
#ifdef VALK_DEBUG
                if (index >= this->size()) {
                    throw std::out_of_range("BinarySpan::go_to");
                }
#endif
                this->index = index;
            }

            template <typename T, std::endian e = std::endian::little>
                requires binary_deserializable<T, e, supports_resize>
            constexpr std::expected<
                T, BinaryStreamError<typename BinaryDeserializable<T, e>::error_type>>
            deserialize() {
                if constexpr (supports_resize) {
                    auto span = this->to_static_size();
                    return BinaryDeserializable<T, e>::deserialize(span);
                } else {
                    return BinaryDeserializable<T, e>::deserialize(std::ref(*this));
                }
            }

            [[nodiscard]] constexpr bool enough_space_for(const size_t required) const {
                return this->remaining() >= required;
            }

            constexpr void reset_index() { this->index = 0; }

            constexpr uint8_t next() {
#ifdef VALK_DEBUG
                if (!this->enough_space_for(1)) {
                    throw std::out_of_range("BinarySpan::next");
                }
#endif
                return this->backing_type[this->index++];
            }

            [[nodiscard]] constexpr uint8_t peek() const {
#ifdef VALK_DEBUG
                if (!this->enough_space_for(1)) {
                    throw std::out_of_range("BinarySpan::next");
                }
#endif
                return this->backing_type[this->index];
            }
        };

        constexpr void BinarySpanStorage<true>::skip_n(const size_t count) {
            if (!this->cast().enough_space_for(count)) {
                this->backing_type.resize(this->cast().size() + count);
            }
            this->index += count;
        }

        constexpr void BinarySpanStorage<true>::write_byte(const uint8_t byte) {
            if (!this->cast().enough_space_for(1)) {
                this->backing_type.reserve(this->cast().size() * 2 + 1);
                this->backing_type.resize(this->cast().size() + 1);
            }
            this->backing_type[this->index++] = byte;
        }

        constexpr BinarySpan<false> BinarySpanStorage<true>::to_static_size() const {
            const std::span span{this->backing_type};
            auto            result = BinarySpan<false>{span};
            result.go_to(this->index);

            return result;
        }

        constexpr BinarySpan<true>& BinarySpanStorage<true>::cast() {
            return *static_cast<BinarySpan<true>*>(this);
        }

        constexpr void BinarySpanStorage<false>::skip_n(const size_t count) {
#ifdef VALK_DEBUG
            if (!this->cast().enough_space_for(count)) {
                throw std::out_of_range("BinarySpan::next");
            }
#endif
            this->index += count;
        }

        constexpr BinarySpan<false>& BinarySpanStorage<false>::cast() {
            return static_cast<BinarySpan<false>&>(*this);
        }

    } // namespace detail

    using NonOwnedBinarySpan = detail::BinarySpan<false>;
    using OwnedBinarySpan    = detail::BinarySpan<true>;

    static_assert([] consteval {
        static constexpr auto arr = std::bit_cast<std::array<uint8_t, 4>>(0x1);

        NonOwnedBinarySpan span{arr};

        int result = 0;
        for (int x = 0; x < 4; ++x) {
            result |= static_cast<int>(span.next()) << x * 8;
        }

        return result;
    }() == 0x1);

    template <std::integral int_type, std::endian endian>
    struct BinarySerializable<int_type, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<void, BinaryStreamError<error_type>>
        serialize(const int_type& val, OwnedBinarySpan& stream) {
            const int_type value = [&] {
                if constexpr (endian == std::endian::little) {
                    return val;
                }
                return std::byteswap(val);
            }();

            for (size_t x = 0; x < sizeof(int_type); x++) {
                stream.write_byte(static_cast<uint8_t>(value >> x * 8));
            }
            return std::expected<void, BinaryStreamError<error_type>>{};
        }
    };

    template <std::integral int_type, std::endian endian>
    struct BinaryDeserializable<int_type, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<int_type, BinaryStreamError<error_type>>
        deserialize(NonOwnedBinarySpan& span) {
            if (!span.enough_space_for(sizeof(int_type))) {
                return std::unexpected{BinaryStreamError<error_type>::out_of_space()};
            }

            int_type result = 0;
            for (size_t x = 0; x < sizeof(int_type); ++x) {
                result |= static_cast<int_type>(span.next()) << x * 8;
            }

            if constexpr (endian == std::endian::big) {
                return std::byteswap(result);
            }
            return result;
        }
    };

    // static_assert([] consteval {
    //     OwnedBinarySpan span{};
    //
    //     constexpr int value = 1;
    //
    //     span.serialize(value).value();
    //     span.reset_index();
    //     const auto result = span.deserialize<int>().value();
    //
    //     return result;
    // }() == 0x1);

    static_assert([] consteval {
        OwnedBinarySpan span{};
        span.write_byte(1);
        span.write_byte(1);
        span.write_byte(1);
        return span.size();
    }() == 3);

    template <std::floating_point float_type, std::endian endian>
    struct BinarySerializable<float_type, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<void, BinaryStreamError<error_type>>
        serialize(const float_type& val, OwnedBinarySpan& stream) {
            using int_type = std::conditional_t<sizeof(float_type) == 4, uint32_t, uint64_t>;

            int_type bits = std::bit_cast<int_type>(val);
            if constexpr (endian == std::endian::big) {
                bits = std::byteswap(bits);
            }

            for (size_t x = 0; x < sizeof(int_type); x++) {
                stream.write_byte(static_cast<uint8_t>(bits >> x * 8));
            }
            return std::expected<void, BinaryStreamError<error_type>>{};
        }
    };

    template <std::floating_point float_type, std::endian endian>
    struct BinaryDeserializable<float_type, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<float_type, BinaryStreamError<error_type>>
        deserialize(NonOwnedBinarySpan& span) {
            using int_type = std::conditional_t<sizeof(float_type) == 4, uint32_t, uint64_t>;
            if (!span.enough_space_for(sizeof(float_type))) {
                return std::unexpected{BinaryStreamError<error_type>::out_of_space()};
            }

            int_type result = 0;
            for (size_t x = 0; x < sizeof(int_type); ++x) {
                result |= static_cast<int_type>(span.next()) << x * 8;
            }

            if constexpr (endian == std::endian::big) {
                result = std::byteswap(result);
            }

            float_type value = std::bit_cast<float_type>(result);
            return value;
        }
    };

    // static_assert([] consteval {
    //     OwnedBinarySpan span{};
    //
    //     constexpr float value = 1.0f;
    //
    //     span.serialize(value).value();
    //     span.reset_index();
    //     const auto result = span.deserialize<float>().value();
    //
    //     return result;
    // }() == 1.0f);

    template <typename T, std::endian endian> struct BinarySerializable<VarInt<T>, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<void, BinaryStreamError<error_type>>
        serialize(const VarInt<T>& var, OwnedBinarySpan& stream) {
            using U = std::make_unsigned_t<T>;
            U value = zigzag_encode(var.value);

            while (true) {
                if ((value & ~0x7Fu) == 0) {
                    stream.write_byte(static_cast<uint8_t>(value));
                    break;
                };

                stream.write_byte(static_cast<uint8_t>((value & 0x7F) | 0x80));
                value >>= 7;
            };

            return std::expected<void, BinaryStreamError<error_type>>{};
        };
    };

    template <typename T, std::endian endian> struct BinaryDeserializable<VarInt<T>, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<VarInt<T>, BinaryStreamError<error_type>>
        deserialize(NonOwnedBinarySpan& span) {
            using U    = std::make_unsigned_t<T>;
            U   result = 0;
            int shift  = 0;

            while (true) {
                const uint8_t byte  = span.next();
                result             |= (static_cast<U>(byte & 0x7F) << shift);
                if ((byte & 0x80) == 0)
                    break;

                shift += 7;
                if (shift >= static_cast<int>(sizeof(T) * 8)) {
                    return std::unexpected{BinaryStreamError<error_type>::malformed()};
                }
            }

            return VarInt<T>{zigzag_decode<T>(result)};
        }
    };

    // static_assert([] consteval {
    //     OwnedBinarySpan span{};
    //
    //     constexpr VarInt<> value{1};
    //
    //     span.serialize(value).value();
    //     span.reset_index();
    //     const auto result = span.deserialize<VarInt<>>().value();
    //
    //     return result;
    // }() == VarInt<>{0x1});

    template <typename StringType, std::endian endian>
        requires requires { string_length_encoding<StringType>::value; }
    struct BinarySerializable<StringType, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<void, BinaryStreamError<error_type>>
        serialize(const StringType& value, OwnedBinarySpan& stream) {
            const std::string& str = value;
            if constexpr (string_length_encoding<StringType>::value ==
                          StringLengthEncoding::VarInt) {
                auto result = stream.serialize<VarInt<>, endian>(
                    VarInt<>(static_cast<uint32_t>(str.size()))
                );

                if (!result) {
                    return result;
                }
            } else {
                auto result =
                    stream.serialize<uint32_t, endian>(static_cast<uint32_t>(str.size()));

                if (!result) {
                    return result;
                }
            }

            for (const unsigned char c : str) {
                stream.write_byte(c);
            }

            return std::expected<void, BinaryStreamError<error_type>>{};
        }
    };

    template <typename StringType, std::endian endian>
        requires requires { string_length_encoding<StringType>::value; }
    struct BinaryDeserializable<StringType, endian> {
        using error_type = std::monostate;

        static constexpr std::expected<StringType, BinaryStreamError<error_type>>
        deserialize(NonOwnedBinarySpan& span) {
            size_t length = 0;

            if constexpr (string_length_encoding<StringType>::value ==
                          StringLengthEncoding::VarInt) {
                auto len_result = span.deserialize<VarInt<>, endian>();
                if (!len_result) {
                    return std::unexpected{len_result.error()};
                }

                length = static_cast<size_t>(len_result.value());
            } else {
                auto len_result = span.deserialize<uint32_t, endian>();
                if (!len_result) {
                    return std::unexpected{len_result.error()};
                }

                length = static_cast<size_t>(len_result.value());
            }

            if (!span.enough_space_for(length)) {
                return std::unexpected{BinaryStreamError<error_type>::out_of_space()};
            }

            std::string result;
            result.resize(length);

            for (size_t i = 0; i < length; ++i) {
                result[i] = static_cast<char>(span.next());
            }

            return result;
        }
    };

    // static_assert([] consteval {
    //     OwnedBinarySpan span{};
    //
    //     constexpr static VarIntString value{"hello!"};
    //
    //     span.serialize(value).value();
    //     span.reset_index();
    //     const std::string result = span.deserialize<VarIntString>().value();
    //
    //     return result;
    // }() == "hello!");

    static_assert(binary_serializeable<std::string, std::endian::little>);
    static_assert(binary_deserializable<std::string, std::endian::little>);
    static_assert(binary_serializeable<VarIntString, std::endian::little>);
    static_assert(binary_deserializable<VarIntString, std::endian::little>);

    static_assert(binary_serializeable<VarInt<>, std::endian::little>);
    static_assert(binary_deserializable<VarInt<>, std::endian::little>);
    static_assert(binary_serializeable<VarInt<int32_t>, std::endian::little>);
    static_assert(binary_deserializable<VarInt<int32_t>, std::endian::little>);
    static_assert(binary_serializeable<VarInt<int64_t>, std::endian::little>);
    static_assert(binary_deserializable<VarInt<int64_t>, std::endian::little>);
    static_assert(binary_serializeable<VarInt<uint64_t>, std::endian::little>);
    static_assert(binary_deserializable<VarInt<uint64_t>, std::endian::little>);

    static_assert(binary_serializeable<float, std::endian::little>);
    static_assert(binary_deserializable<float, std::endian::little>);
    static_assert(binary_serializeable<double, std::endian::little>);
    static_assert(binary_deserializable<double, std::endian::little>);

    static_assert(binary_serializeable<int32_t, std::endian::little>);
    static_assert(binary_deserializable<int32_t, std::endian::little>);
    static_assert(binary_serializeable<uint32_t, std::endian::little>);
    static_assert(binary_deserializable<uint32_t, std::endian::little>);
    static_assert(binary_serializeable<int64_t, std::endian::little>);
    static_assert(binary_deserializable<int64_t, std::endian::little>);
    static_assert(binary_serializeable<uint64_t, std::endian::little>);
    static_assert(binary_deserializable<uint64_t, std::endian::little>);
    static_assert(binary_serializeable<int16_t, std::endian::little>);
    static_assert(binary_deserializable<int16_t, std::endian::little>);
    static_assert(binary_serializeable<uint16_t, std::endian::little>);
    static_assert(binary_deserializable<uint16_t, std::endian::little>);
    static_assert(binary_serializeable<int8_t, std::endian::little>);
    static_assert(binary_deserializable<int8_t, std::endian::little>);
    static_assert(binary_serializeable<uint8_t, std::endian::little>);
    static_assert(binary_deserializable<uint8_t, std::endian::little>);

} // namespace valk