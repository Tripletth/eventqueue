#pragma once

#include "event.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

template <typename EventType>
class dispatcher {
    using callback_t = std::function<void(const event<EventType>*)>;
public:
    dispatcher() = default;

    void subscribe(EventType e, callback_t c) {
        mapping[e].emplace_back(std::move(c));
    }
    void dispatch(const event<EventType>* e) const {
        if (mapping.contains(e->type)) {
            auto callbacks = mapping.at(e->type);
            std::for_each(callbacks.begin(), callbacks.end(), [&e](auto& callback){
                callback(e);
            });
        }
    }

private:
    std::unordered_map<EventType, std::vector<callback_t>> mapping{};
};
