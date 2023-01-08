#pragma once

#include <memory>
#include <mutex>
#include <queue>

class event_queue {
public:
    enum class event_type {MESSAGE, INPUT};
    struct event {
        event(event_type t) : type{t} {}
        event_type type;
    };

    event_queue();

    template <typename T, typename ... Args>
    void push(Args ... args) {
        std::lock_guard<std::mutex> lock{mutex};
        events.emplace(std::make_unique<T>(args...));
    }

    std::unique_ptr<event> pop();
    bool empty();
private:
    mutable std::mutex mutex;
    std::queue<std::unique_ptr<event>> events;
};
