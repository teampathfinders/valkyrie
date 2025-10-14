#pragma once
#include <concepts>

namespace valk::event {

    struct BaseEvent {};

    struct CancelableEvent : BaseEvent {};

    template <typename T>
    concept EventConcept = std::derived_from<T, BaseEvent>;

    template <typename> struct IsCancelable : std::false_type {};

    template <typename T>
        requires std::derived_from<T, CancelableEvent>
    struct IsCancelable<T> : std::true_type {};

} // namespace valk::event
