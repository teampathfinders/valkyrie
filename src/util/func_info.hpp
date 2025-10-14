#pragma once
#include <tuple>

namespace valk::meta {
    template <typename T> struct FuncInfo;

    template <typename... Ts> struct ArgInfo {
        template <typename Callback>
        static consteval bool for_each_type_assert(Callback&& callback) {
            return (... && callback(std::type_identity<Ts>{}));
        }

        template <typename ReturnType, typename Callback, std::size_t... Is>
        static consteval std::array<ReturnType, sizeof...(Ts)>
        for_each_type_impl(Callback const& callback, std::index_sequence<Is...>) {
            return {callback(std::type_identity<Ts>{})...};
        }

        template <typename ReturnType, typename Callback>
        static consteval std::array<ReturnType, sizeof...(Ts)>
        for_each_type(const Callback& callback) {
            return for_each_type_impl<ReturnType, Callback>(
                callback, std::index_sequence_for<Ts...>{}
            );
        }
    };

    template <typename Return, typename... Args> struct FuncInfo<Return (*)(Args...)> {
        using return_type               = Return;
        using args_type                 = std::tuple<Args...>;
        using ptr                       = Return (*)(Args...);
        constexpr static auto arg_count = std::tuple_size_v<args_type>;

        template <size_t index> // gets arg at this index
        using arg_at   = std::tuple_element_t<index, args_type>;
        using arg_info = ArgInfo<Args...>;
    };

#define MemberPtr(extension)                                                                   \
    template <typename Return, typename Class, typename... Args>                               \
    struct FuncInfo<Return (Class::*)(Args...) extension> {                                    \
        using return_type               = Return;                                              \
        using args_type                 = std::tuple<Args...>;                                 \
        using class_type                = Class;                                               \
        using ptr                       = Return (Class::*)(Args...);                          \
        constexpr static auto arg_count = std::tuple_size_v<args_type>;                        \
                                                                                               \
        template <size_t index> using arg_at = std::tuple_element_t<index, args_type>;         \
        using arg_info                       = ArgInfo<Args...>;                               \
    } // namespace cubix::meta

    MemberPtr();
    MemberPtr(&);
    MemberPtr(&&);
    MemberPtr(const);
    MemberPtr(const&);
    MemberPtr(const&&);
    MemberPtr(noexcept);
    MemberPtr(& noexcept);
    MemberPtr(&& noexcept);
    MemberPtr(const noexcept);
    MemberPtr(const& noexcept);
    MemberPtr(const&& noexcept);
    template <typename T> struct FuncInfo : FuncInfo<decltype(&T::operator())> {};

    namespace test {
        inline void fn(int, float, const char*) {
            static_assert(std::same_as<FuncInfo<decltype(&fn)>::arg_at<0>, int>);
        }

        inline void fn1(int&) {
            static_assert(std::same_as<std::decay_t<FuncInfo<decltype(&fn1)>::arg_at<0>>, int>);
        }

    } // namespace test
} // namespace valk::meta
#undef MemberPtr