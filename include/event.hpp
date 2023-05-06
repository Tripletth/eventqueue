#pragma once

template <typename EventType>
struct event {
    event(EventType t) : type{t} {}
    EventType type;
};
