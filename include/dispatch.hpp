#pragma once

#include "eventqueue.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

class dispatcher {
public:
    dispatcher() = default;

    using callback_t = std::function<void(event_queue::event*)>;
    void subscribe(event_queue::event_type e, callback_t c);

    void dispatch(event_queue::event* e);
private:
    std::unordered_map<event_queue::event_type, std::vector<callback_t>> mapping{};
};
