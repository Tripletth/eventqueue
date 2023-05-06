#pragma once

#include "event.hpp"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template <typename EventType>
class event_queue {
public:
    event_queue()
    : cv{}, mutex{}, events{}
    {}

    template <typename T, typename ... Args>
    void push(Args ... args) {
        std::lock_guard<std::mutex> lock{mutex};
        events.emplace(std::make_unique<T>(args...));
        cv.notify_one();
    }

    std::unique_ptr<event<EventType>> pop() {
        std::lock_guard<std::mutex> lock{mutex};
        if (!events.empty()) {
            std::unique_ptr<event<EventType>> e = std::move(events.front());
            events.pop();
            return e;
        }
        return nullptr;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock{mutex};
        return events.empty();
    }

    bool wait_for_event() {
        std::unique_lock<std::mutex> lock{mutex};
        cv.wait(lock, [this]{return !events.empty();});
        return !events.empty();
    }

private:
    std::condition_variable cv;
    mutable std::mutex mutex;
    std::queue<std::unique_ptr<event<EventType>>> events;
};
