#pragma once
#include "hash.hpp"
#include "strong_hash.hpp"
#include <algorithm>
#include <limits>
#include <string>
#include <string_view>

namespace valk {

    // I'll be real, I was lazy, AI wrote this whole wrapper

    class HashedString {
    public:
        using value_type             = char;
        using size_type              = std::size_t;
        using difference_type        = std::ptrdiff_t;
        using const_reference        = const value_type&;
        using const_pointer          = const value_type*;
        using const_iterator         = std::string::const_iterator;
        using const_reverse_iterator = std::string::const_reverse_iterator;

        constexpr HashedString()                               = default;
        constexpr HashedString(const HashedString&)            = default;
        constexpr HashedString(HashedString&&)                 = default;
        constexpr HashedString& operator=(const HashedString&) = default;
        constexpr HashedString& operator=(HashedString&&)      = default;

        constexpr explicit HashedString(const std::string& str) : string(str) { rehash(); }
        constexpr explicit HashedString(std::string&& str) : string(std::move(str)) {
            rehash();
        }
        constexpr explicit HashedString(const char* str) : string(str) { rehash(); }
        constexpr explicit HashedString(const char* str, size_type len) : string(str, len) {
            rehash();
        }
        constexpr explicit HashedString(std::string_view str) : string(std::string(str)) {
            rehash();
        }

        constexpr HashedString& assign(std::string_view sv) {
            if (string != sv) {
                string.assign(sv);
                rehash();
            }
            return *this;
        }
        constexpr HashedString& assign(const char* s) {
            if (string != s) {
                string.assign(s);
                rehash();
            }
            return *this;
        }
        constexpr HashedString& assign(const char* s, size_type n) {
            if (string != std::string_view(s, n)) {
                string.assign(s, n);
                rehash();
            }
            return *this;
        }
        template <class It> constexpr HashedString& assign_range(It first, It last) {
            std::string tmp(first, last);
            if (string != tmp) {
                string.swap(tmp);
                rehash();
            }
            return *this;
        }

        constexpr std::allocator<char> get_allocator() const { return string.get_allocator(); }

        constexpr const_reference at(size_type pos) const { return string.at(pos); }
        constexpr const_reference operator[](size_type pos) const { return string[pos]; }
        constexpr const_reference front() const { return string.front(); }
        constexpr const_reference back() const { return string.back(); }
        constexpr const_pointer   data() const noexcept { return string.data(); }
        constexpr const char*     c_str() const noexcept { return string.c_str(); }
        constexpr                 operator std::string_view() const noexcept {
            return std::string_view(string);
        }

        constexpr const_iterator         begin() const noexcept { return string.begin(); }
        constexpr const_iterator         cbegin() const noexcept { return string.cbegin(); }
        constexpr const_iterator         end() const noexcept { return string.end(); }
        constexpr const_iterator         cend() const noexcept { return string.cend(); }
        constexpr const_reverse_iterator rbegin() const noexcept { return string.rbegin(); }
        constexpr const_reverse_iterator crbegin() const noexcept { return string.crbegin(); }
        constexpr const_reverse_iterator rend() const noexcept { return string.rend(); }
        constexpr const_reverse_iterator crend() const noexcept { return string.crend(); }

        constexpr bool      empty() const noexcept { return string.empty(); }
        constexpr size_type size() const noexcept { return string.size(); }
        constexpr size_type length() const noexcept { return string.length(); }
        constexpr size_type max_size() const noexcept { return string.max_size(); }
        constexpr void      reserve(size_type n) { string.reserve(n); }
        constexpr size_type capacity() const noexcept { return string.capacity(); }
        constexpr void      shrink_to_fit() { string.shrink_to_fit(); }

        constexpr void clear() {
            if (!string.empty()) {
                string.clear();
                rehash();
            }
        }
        constexpr HashedString& insert(size_type pos, std::string_view sv) {
            if (!sv.empty()) {
                string.insert(pos, sv);
                rehash();
            }
            return *this;
        }
        template <class It>
        constexpr HashedString& insert_range(const_iterator pos, It first, It last) {
            if (first != last) {
                string.insert(pos, first, last);
                rehash();
            }
            return *this;
        }
        constexpr HashedString& erase(size_type pos = 0, size_type n = std::string::npos) {
            if (pos < string.size()) {
                string.erase(pos, n);
                rehash();
            }
            return *this;
        }
        constexpr void push_back(char c) {
            string.push_back(c);
            rehash();
        }
        constexpr void pop_back() {
            string.pop_back();
            rehash();
        }
        constexpr HashedString& append(std::string_view sv) {
            if (!sv.empty()) {
                string.append(sv);
                rehash();
            }
            return *this;
        }
        template <class It> constexpr HashedString& append_range(It first, It last) {
            if (first != last) {
                string.append(first, last);
                rehash();
            }
            return *this;
        }
        constexpr HashedString& operator+=(std::string_view sv) {
            if (!sv.empty()) {
                string += sv;
                rehash();
            }
            return *this;
        }
        constexpr HashedString& replace(size_type pos, size_type count, std::string_view sv) {
            string.replace(pos, count, sv);
            rehash();
            return *this;
        }
        template <class It>
        constexpr HashedString&
        replace_with_range(const_iterator first, const_iterator last, It f2, It l2) {
            string.replace(first, last, f2, l2);
            rehash();
            return *this;
        }
        constexpr size_type copy(char* dest, size_type count, size_type pos = 0) const {
            return string.copy(dest, count, pos);
        }
        constexpr void resize(size_type count) {
            if (count != string.size()) {
                string.resize(count);
                rehash();
            }
        }
        constexpr void resize(size_type count, char c) {
            if (count != string.size()) {
                string.resize(count, c);
                rehash();
            }
        }
        template <class Op> constexpr void resize_and_overwrite(size_type n, Op op) {
            string.resize_and_overwrite(n, op);
            rehash();
        }
        constexpr void swap(HashedString& other) noexcept {
            string.swap(other.string);
            std::swap(string_hash, other.string_hash);
        }

        constexpr size_type find(std::string_view sv, size_type pos = 0) const {
            return string.find(sv, pos);
        }
        constexpr size_type
        rfind(std::string_view sv, size_type pos = std::string::npos) const {
            return string.rfind(sv, pos);
        }
        constexpr size_type find_first_of(std::string_view sv, size_type pos = 0) const {
            return string.find_first_of(sv, pos);
        }
        constexpr size_type find_first_not_of(std::string_view sv, size_type pos = 0) const {
            return string.find_first_not_of(sv, pos);
        }
        constexpr size_type
        find_last_of(std::string_view sv, size_type pos = std::string::npos) const {
            return string.find_last_of(sv, pos);
        }
        constexpr size_type
        find_last_not_of(std::string_view sv, size_type pos = std::string::npos) const {
            return string.find_last_not_of(sv, pos);
        }

        constexpr int  compare(std::string_view sv) const { return string.compare(sv); }
        constexpr bool starts_with(std::string_view sv) const { return string.starts_with(sv); }
        constexpr bool ends_with(std::string_view sv) const { return string.ends_with(sv); }
        constexpr bool contains(std::string_view sv) const {
            return string.find(sv) != std::string::npos;
        }
        constexpr HashedString
        substr(size_type pos = 0, size_type n = std::string::npos) const {
            return HashedString(string.substr(pos, n));
        }

        constexpr bool operator==(const HashedString& other) const {
            return compare(other) == 0;
        }
        constexpr bool operator!=(const HashedString& other) const { return !(*this == other); }

        constexpr SizeTHash hash() const { return string_hash; }

        constexpr bool operator<(const HashedString& other) const {
#ifdef CUBIX_SAFE_STRINGS
            return string < other.string; // lexicographical ordering
#else
            return string_hash < other.string_hash; // hash-based ordering
#endif
        }

        constexpr bool operator>(const HashedString& other) const {
#ifdef CUBIX_SAFE_STRINGS
            return string > other.string; // lexicographical ordering
#else
            return string_hash > other.string_hash; // hash-based ordering
#endif
        }

    private:
        constexpr void rehash() { string_hash = crypto::fnv164_hash_mojang(string); }
        constexpr int  compare(const HashedString& other) const {
            if (string_hash != other.string_hash) return 1;
#ifdef VALK_SAFE_STRINGS
            return string == other.string ? 0 : 1;
#else
            return 0;
#endif
        }

        std::string string{};
        SizeTHash   string_hash{};
    };

} // namespace valk

namespace valk {
    constexpr bool test_constructors_and_assignment() {
        constexpr static HashedString s1("hello");
        constexpr static HashedString s2(std::string_view("world"));
        HashedString                  s3("test");
        s3.assign("new");
        s3.assign(std::string_view("zzz"));
        return s1 == HashedString("hello") && s2 != s1 && s3 == HashedString("zzz");
    }

    constexpr bool test_hash_consistency() {
        HashedString s("abc");
        auto         h1 = s.hash();
        s.assign("abc");
        auto h2 = s.hash();
        s.assign("abcd");
        auto h3 = s.hash();
        return h1 == h2 && h1 != h3;
    }

    constexpr bool test_element_access() {
        constexpr static HashedString s("hello");
        return s[0] == 'h' && s.at(1) == 'e' && s.front() == 'h' && s.back() == 'o';
    }

    constexpr bool test_capacity_and_size() {
        HashedString s("abc");
        s.clear();
        return s.empty() && s.size() == 0;
    }

    constexpr bool test_append_insert_erase() {
        HashedString s("abc");
        s.append("def");
        s.insert(3, "XYZ");
        s.erase(3, 3);
        s.pop_back();
        s.push_back('!');
        return s == HashedString("abcde!");
    }

    constexpr bool test_replace_and_substr() {
        HashedString s("abcdef");
        s.replace(2, 2, "ZZ");
        auto sub = s.substr(2, 2);
        return s == HashedString("abZZef") && sub == HashedString("ZZ");
    }

    constexpr bool test_find_search() {
        HashedString s("abracadabra");
        return s.find("bra") == 1 && s.rfind("bra") == 8 && s.find_first_of("c") == 4 &&
               s.find_first_not_of("a") == 1 && s.find_last_of("a") == 10 &&
               s.find_last_not_of("a") == 9;
    }

    constexpr bool test_compare_starts_ends_contains() {
        HashedString s("foobar");
        return s.compare("foobar") == 0 && s.starts_with("foo") && s.ends_with("bar") &&
               s.contains("oba") && !s.contains("zzz");
    }

    constexpr bool test_swap_copy_resize() {
        HashedString a("one");
        HashedString b("two");
        a.swap(b);
        char buf[10]{};
        auto n = a.copy(buf, 3);
        a.resize(2);
        a.resize(4, 'x');
        return a == HashedString("twxx") && b == HashedString("one") && n == 3;
    }

    // Compile-time assertions
    static_assert(test_constructors_and_assignment());
    static_assert(test_hash_consistency());
    static_assert(test_element_access());
    static_assert(test_capacity_and_size());
    static_assert(test_append_insert_erase());
    static_assert(test_replace_and_substr());
    static_assert(test_find_search());
    static_assert(test_compare_starts_ends_contains());
    static_assert(test_swap_copy_resize());
} // namespace valk

namespace std {
    template <> struct hash<valk::HashedString> {
        size_t operator()(const valk::HashedString& str) const noexcept {
            return str.hash().get_hash();
        }
    };
} // namespace std
