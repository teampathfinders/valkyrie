#include "event/base_event.hpp"
#include "event/event_provider.hpp"

#include <gtest/gtest.h>

namespace valk {
    struct SimpleEvent : event::BaseEvent {};
    struct CancelEvent : event::CancelableEvent {};
} // namespace valk

class State {
public:
    int fired_count{0};

    void simple_event(valk::SimpleEvent&) { ++this->fired_count; }

    bool always_cancel(valk::CancelEvent&) {
        ++this->fired_count;
        return true;
    }
};

TEST(EventProvider, EventsFired) {

    State state{};
    valk::event::EventProvider::add_listener<&State::simple_event>(&state);

    valk::SimpleEvent event{};
    valk::event::EventProvider::fire_event(event);
    ASSERT_EQ(state.fired_count, 1);
}

TEST(EventProvider, EventsCancled) {

    State state{};
    valk::event::EventProvider::add_listener<&State::always_cancel>(&state);
    valk::event::EventProvider::add_listener<&State::always_cancel>(&state);

    valk::CancelEvent event{};
    valk::event::EventProvider::fire_event(event);
    ASSERT_EQ(state.fired_count, 1);
}

TEST(EventProvider, EventsDeaf) {

    State state{};
    valk::event::EventProvider::add_listener<&State::simple_event>(&state);
    valk::event::EventProvider::add_listener<&State::simple_event>(&state);
    const auto id = valk::event::EventProvider::add_listener<&State::simple_event>(&state);
    valk::event::EventProvider::deafen(id);
    valk::SimpleEvent event{};
    valk::event::EventProvider::fire_event(event);
    ASSERT_EQ(state.fired_count, 2);
}

TEST(EventProvider, EventQueue) {
    valk::SimpleEvent event{};
    valk::event::EventProvider::queue_event(0, event);
    valk::event::EventProvider::queue_event(0, event);
    valk::event::EventProvider::queue_event(0, event);
    valk::event::EventProvider::queue_event(0, event);
    valk::event::EventProvider::queue_event(0, event);

    State state{};
    valk::event::EventProvider::add_listener<&State::simple_event>(&state);
    valk::event::EventProvider::fire_queued_events(0);

    ASSERT_EQ(state.fired_count, 5);
}
