#include "eventqueue.hpp"

#include <mutex>

event_queue::event_queue()
: mutex{}, events{}
{}

std::unique_ptr<event_queue::event> event_queue::pop() {
    if (!empty()) {
        std::lock_guard<std::mutex> lock{mutex};
        std::unique_ptr<event> e = std::move(events.front());
        events.pop();
        return e;
    }
    return nullptr;
}

bool event_queue::empty() {
    std::lock_guard<std::mutex> lock{mutex};

    return events.empty();
}
