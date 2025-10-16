#pragma once

#include <format>
#include <util/binary_span.hpp>

namespace valk::nbt {

    struct EndianBase {};

    template <typename T>
    concept Endianness = std::is_base_of_v<EndianBase, T>;

    struct LittleEndian : EndianBase {};
    struct BigEndian : EndianBase {};
    struct NetworkEndian : EndianBase {};

    enum class Variant {
        End,
        Byte,
        Short,
        Int,
        Long,
        Float,
        Double,
        String,
        List,
        Compound,
        ByteArray,
        IntArray,
        LongArray,
        Unassigned
    };

    template <typename> struct TagMeta {
        using OwnedT = void;
        using RefT   = void;

        static constexpr uint8_t ID = 0xFF;
    };

    template <typename T>
    concept TagType = !std::is_same_v<typename TagMeta<T>::OwnedT, void>;

    struct End {};

    template <> struct TagMeta<End> {
        static constexpr uint8_t ID = 0x00;
    };

    struct Byte {};

    template <> struct TagMeta<Byte> {
        using OwnedT = uint8_t;
        using RefT   = uint8_t;

        static constexpr uint8_t ID = 0x01;
    };

    struct Short {};

    template <> struct TagMeta<Short> {
        using OwnedT = uint16_t;
        using RefT   = uint16_t;

        static constexpr uint8_t ID = 0x02;
    };

    struct Int {};

    template <> struct TagMeta<Int> {
        using OwnedT = int32_t;
        using RefT   = int32_t;

        static constexpr uint8_t ID = 0x03;
    };

    struct Long {};

    template <> struct TagMeta<Long> {
        using StorageT = int64_t;
        using RefT     = int64_t;

        static constexpr uint8_t ID = 0x04;
    };

    struct Float {};

    template <> struct TagMeta<Float> {
        using OwnedT = float;
        using RefT   = float;

        static constexpr uint8_t ID = 0x05;
    };

    struct Double {};

    template <> struct TagMeta<Double> {
        using OwnedT = double;
        using RefT   = double;

        static constexpr uint8_t ID = 0x06;
    };

    struct String {};

    template <> struct TagMeta<String> {
        using OwnedT = std::string;
        using RefT   = std::string_view;

        static constexpr uint8_t ID = 0x08;
    };

    class Root {
    public:
        template <typename T>
            requires std::is_base_of_v<Root, T>
        T* as() noexcept {
            return dynamic_cast<T*>(this);
        }
    };

    template <TagType T, bool Owned = true> class List : public Root {
    public:
        // If Copied == true, use the owned variant of the tag, otherwise use ref.
        using StorageT =
            std::conditional_t<Owned, typename TagMeta<T>::OwnedT, typename TagMeta<T>::RefT>;
        using VecT = std::conditional_t<Owned, std::vector<StorageT>, StorageT*>;

        List(const List& other) = delete;
        List(List&& other) noexcept { m_values = std::move(other.m_values); }

        /// Returns whether this list owns its data.
        static constexpr bool is_owned() noexcept { return Owned; }

        /// Accesses a constant element in the list *without* performing bounds checks.
        const StorageT& operator[](size_t index) const { return m_values[index]; }

        /// Accesses a mutable element in the list *without* performing bounds checks.
        StorageT& operator[](size_t index) { return m_values[index]; }

        /// Accesses a constant element in the list.
        /// Unlike the [] operator, this function throws a `std::out_of_range` exception
        /// if out-of-bounds access occurs.
        const StorageT& at(size_t index) const {
            // checking bounds manually instead of using .at() because `m_values` might be a
            // pointer.
            if (index >= size()) {
                throw std::out_of_range("index out of range");
            }
            return m_values.at(index);
        }

        /// Mutably accesses an element in the list.
        /// Unlike the [] operator, this function throws a `std::out_of_range` exception
        /// if out-of-bounds access occurs.
        StorageT& at(size_t index) {
            // checking bounds manually instead of using .at() because `m_values` might be a
            // pointer.
            if (index >= size()) {
                throw std::out_of_range("index out of range");
            }
            return m_values.at(index);
        }

        size_t size() const noexcept {
            if constexpr (Owned) {
                return m_values.size();
            } else {
                return std::get<0>(m_values);
            }
        }

        void push(StorageT value) {
            if constexpr (!is_owned()) {
                static_assert(false, "Cannot push to non-owned list");
            }
            m_values.push_back(std::move(value));
        }

    private:
        VecT m_values;
    };

    template <TagType U, bool O> struct TagMeta<List<U, O>> {
        using OwnedT = List<U, true>;
        using RefT   = List<U, false>;

        static constexpr uint8_t ID = 0x09;
    };

    class Compound : public Root {
    public:
        Compound() {}

        template <TagType T>
        std::add_const_t<std::add_lvalue_reference_t<typename TagMeta<T>::RefT>>
        get(const char*) const {}

        template <TagType T>
        std::add_lvalue_reference_t<typename TagMeta<T>::OwnedT> get(const char* key) {
            if (key == nullptr) {
                throw std::runtime_error("yes");
            }
            throw std::runtime_error("yes");
        }

    private:
    };

    template <> struct TagMeta<Compound> {
        using OwnedT = Compound;
        using RefT   = Compound;

        static constexpr uint8_t ID = 0x0a;
    };

    template <bool Owned = false> struct ByteArray : List<Byte, Owned> {};

    template <bool O> struct TagMeta<ByteArray<O>> {
        using OwnedT = std::vector<uint8_t>;
        using RefT   = uint8_t*;

        static constexpr uint8_t ID = 0x07;
    };

    template <bool Owned = false> struct IntArray : List<Int, Owned> {};

    template <bool O> struct TagMeta<IntArray<O>> {
        using OwnedT = std::vector<int32_t>;
        using RefT   = int32_t*;

        static constexpr uint8_t ID = 0x0b;
    };

    template <bool Owned = false> struct LongArray : List<Long, Owned> {};

    template <bool O> struct TagMeta<LongArray<O>> {
        using OwnedT = std::vector<int64_t>;
        using RefT   = int64_t*;

        static constexpr uint8_t ID = 0x0c;
    };

    template <Endianness E, bool SpanOwned>
    ValkExpect<Root> deserialize(detail::BinarySpan<SpanOwned>) {}
} // namespace valk::nbt

using namespace valk::nbt;

void test() {}