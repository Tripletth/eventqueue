#include "dispatch.hpp"

void dispatcher::subscribe(event_queue::event_type e, callback_t c) {
    mapping[e].emplace_back(std::move(c));
}

void dispatcher::dispatch(event_queue::event* e) {
    if (mapping.contains(e->type)) {
        auto callbacks = mapping.at(e->type);
        std::for_each(callbacks.begin(), callbacks.end(), [e](auto&& callback){
            callback(e);
        });
    }
}
