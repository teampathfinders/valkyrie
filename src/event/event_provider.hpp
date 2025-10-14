#pragma once
#include "base_event.hpp"
#include "entt/entt.hpp"
#include "parallel_hashmap/phmap.h"
#include "shared_recursive_mutex/shared_recursive_mutex.hpp"
#include "util/func_info.hpp"

namespace valk::event {

    namespace detail {
        template <auto T>
        using EventReturnType = std::conditional_t<
            std::derived_from<
                std::decay_t<typename meta::FuncInfo<decltype(T)>::template arg_at<0>>,
                CancelableEvent>,
            bool, void>;

        template <bool Cancelable>
        using SimpleReturnType = std::conditional_t<Cancelable, bool, void>;

        template <auto T>
        concept ListenerConcept =
            std::derived_from<
                std::decay_t<typename meta::FuncInfo<decltype(T)>::template arg_at<0>>,
                BaseEvent> &&
            std::is_invocable_r_v<
                EventReturnType<T>, decltype(T),
                std::decay_t<typename meta::FuncInfo<decltype(T)>::template arg_at<0>>&>;

        template <auto T>
        concept ListenerConceptClass =
            std::derived_from<
                std::decay_t<typename meta::FuncInfo<decltype(T)>::template arg_at<0>>,
                BaseEvent> &&
            requires(
                typename meta::FuncInfo<decltype(T)>::class_type*                       obj,
                std::decay_t<typename meta::FuncInfo<decltype(T)>::template arg_at<0>>& evt
            ) {
                { (obj->*T)(evt) } -> std::same_as<EventReturnType<T>>;
            };

    } // namespace detail

    class EventProvider {
    public:
        using EventId                              = entt::id_type;
        constexpr static EventId  INVALID_EVENT_ID = entt::null;
        constexpr static uint32_t InvalidIndex     = std::numeric_limits<uint32_t>::max();

    private:
        class IListenerBase {
        public:
            virtual ~IListenerBase() = default;
        };

        template <EventId, typename Ret, typename Event>
        class SpecialEventListener : public IListenerBase {
        public:
            ~SpecialEventListener() override = default;

            virtual Ret invoke(Event&) = 0;
        };

        template <EventId val, typename Ret, typename Event, typename Class>
        class SpecialEventListenerClass : public SpecialEventListener<val, Ret, Event> {
        public:
            explicit SpecialEventListenerClass(Class* instance) : instance(instance) {}
            ~SpecialEventListenerClass() override = default;

            Ret invoke(Event&) override = 0;

            Class* instance{};
        };

    public:
        struct ListenerId {
            const EventId  event{INVALID_EVENT_ID};
            const uint32_t listener_index{InvalidIndex};
        };

        static EventProvider& instance() {
            static EventProvider instance;
            return instance;
        }

        template <EventConcept T> static void fire_event(T& event_type) {
            auto& self              = instance();
            using return_type       = detail::SimpleReturnType<IsCancelable<T>::value>;
            constexpr auto event_id = entt::type_hash<T>::value();
            using CastType          = SpecialEventListener<event_id, return_type, T>;

            std::shared_lock lock{event_list_mutex};
            auto&            info = self.events[event_id];

            for (auto& listener : info.listeners) {
                if (listener.deaf)
                    continue;
                auto callback = static_cast<CastType&>(*listener.listener);

                if constexpr (IsCancelable<T>::value) {
                    const auto cancel = callback->invoke(event_type);

                    if (cancel) {
                        return;
                    }
                } else {
                    callback->invoke(event_type);
                }
            }
        }

        static void deafen(ListenerId id);
        static void undeafen(ListenerId id);

        template <auto ListenerCallback>
            requires detail::ListenerConcept<ListenerCallback>
        static ListenerId add_listener() {
            using T          = decltype(ListenerCallback);
            using event_type = std::decay_t<typename meta::FuncInfo<T>::template arg_at<0>>;
            constexpr auto event_id = entt::type_hash<event_type>::value();
            auto&          self     = instance();

            class CustomSpecialEventListener
                : public SpecialEventListener<
                      event_id, detail::EventReturnType<ListenerCallback>, event_type> {
            public:
                ~CustomSpecialEventListener() override = default;

                detail::EventReturnType<ListenerCallback> invoke(event_type& event) override {
                    return ListenerCallback(event);
                }
            };

            std::unique_ptr<IListenerBase> listener =
                std::make_unique<CustomSpecialEventListener>();

            std::unique_lock lock{event_list_mutex};
            auto&            info = self.events[event_id];
            const auto       id   = ListenerId{
                        .event          = event_id,
                        .listener_index = static_cast<uint32_t>(info.listeners.size())
            };

            info.listeners.emplace_back(false, std::move(listener));
            return id;
        }

        template <auto ListenerCallback>
            requires detail::ListenerConceptClass<ListenerCallback> &&
                     std::is_member_function_pointer_v<decltype(ListenerCallback)>
        static ListenerId add_listener(
            std::add_const_t<typename meta::FuncInfo<decltype(ListenerCallback)>::class_type>*
                class_instance
        ) {
            return EventProvider::add_listener<ListenerCallback>(
                const_cast<typename meta::FuncInfo<decltype(ListenerCallback)>::class_type*>(
                    class_instance
                )
            );
        }

        template <auto ListenerCallback>
            requires detail::ListenerConceptClass<ListenerCallback> &&
                     std::is_member_function_pointer_v<decltype(ListenerCallback)>
        static ListenerId add_listener(
            typename meta::FuncInfo<decltype(ListenerCallback)>::class_type* class_instance
        ) {
            using T          = decltype(ListenerCallback);
            using event_type = std::decay_t<typename meta::FuncInfo<T>::template arg_at<0>>;
            using class_type = typename meta::FuncInfo<decltype(ListenerCallback)>::class_type;
            constexpr auto event_id = entt::type_hash<event_type>::value();

            auto& self = instance();

            class CustomSpecialEventListener
                : public SpecialEventListenerClass<
                      event_id, detail::EventReturnType<ListenerCallback>, event_type,
                      class_type> {
            public:
                explicit CustomSpecialEventListener(class_type* class_instance)
                    : SpecialEventListenerClass<
                          event_id, detail::EventReturnType<ListenerCallback>, event_type,
                          class_type>(class_instance) {}
                ~CustomSpecialEventListener() override = default;

                detail::EventReturnType<ListenerCallback> invoke(event_type& event) override {
                    return (this->instance->*ListenerCallback)(event);
                }
            };

            std::unique_ptr<IListenerBase> listener =
                std::make_unique<CustomSpecialEventListener>(class_instance);

            std::unique_lock lock{event_list_mutex};
            auto&            info = self.events[event_id];
            const auto       id   = ListenerId{
                        .event          = event_id,
                        .listener_index = static_cast<uint32_t>(info.listeners.size())
            };

            info.listeners.emplace_back(false, std::move(listener));
            return id;
        }

    private:
        struct Listener {
            // It may be worth looking into a way to use entt::poly if I can figure it out
            bool                           deaf{false};
            std::unique_ptr<IListenerBase> listener{};
        };

        struct EventInfo {
            std::vector<Listener> listeners{};
        };

        static inline auto& event_list_mutex =
            mtx::shared_recursive_mutex_t<EventProvider>::instance();
        phmap::node_hash_map<EventId, EventInfo> events{};
    };
} // namespace valk::event
