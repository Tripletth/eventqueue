#include "dispatcher.hpp"
#include "event.hpp"
#include "eventqueue.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <string>
#include <string_view>
#include <thread>

namespace {
std::mutex cout_mutex{};

enum class event_type {
    MESSAGE,
    INPUT,
    OUTPUT
};

struct input_event : public event<event_type> {
    input_event(int i)
    : event{event_type::INPUT}, input{i}
    {}
    int input;
};

struct message_event : public event<event_type> {
    message_event(std::string_view m)
    : event{event_type::MESSAGE}, message{m.begin(), m.end()}
    {}
    std::string message;
};

struct output_event : public event<event_type> {
    output_event(int i)
    : event{event_type::OUTPUT}, output{i}
    {}
    int output;
};

class event_generator {
public:
    event_generator(std::string_view id, event_type t, event_queue<event_type>* queue, dispatcher<event_type>* dispatch)
    :id{id}, type{t}, end{false}, queue{queue}, thread{}
    {}
    event_generator(event_generator&& e)
    : id{std::move(e.id)}, type{e.type}, end{e.end.load()}, queue{e.queue}, thread{std::move(e.thread)}
    {}
    void start() {
        thread = std::thread(&event_generator::loop, this);
    }
    ~event_generator() {
        end = true;
        if (thread.joinable()) {
            thread.join();
        }
    }
    void loop() {
        while (!end.load()) {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 2000));

            {
                std::lock_guard<std::mutex> lck{cout_mutex};
                std::cout << "generator [" << id << "] pushing event." << std::endl;
            }

            static uint64_t cnt{0};
            if (type == event_type::INPUT) {
                queue->push<input_event>(++cnt);
            } else {
                queue->push<message_event>(id + " message_" + std::to_string(++cnt));
            }
        }
    }
private:
    std::string id;
    event_type type;
    std::atomic_bool end;
    event_queue<event_type>* queue;
    std::thread thread;
};
}

int main() {
    constexpr auto nr_of_threads{10};
    std::vector<event_generator> generators{};
    std::srand(std::time(nullptr));

    event_queue<event_type> queue{};
    dispatcher<event_type> dispatch{};

    dispatch.subscribe(event_type::MESSAGE, [](std::unique_ptr<event<event_type>>&& e){
        std::lock_guard<std::mutex> lck{cout_mutex};
        std::cout << "\tmessage event: " << static_cast<message_event*>(e.get())->message << std::endl;
    });
    dispatch.subscribe(event_type::INPUT, [&queue](std::unique_ptr<event<event_type>>&& e){
        std::lock_guard<std::mutex> lck{cout_mutex};
        std::cout << "\t\tinput event: " << static_cast<input_event*>(e.get())->input << ", pushing event. "<< std::endl;
        queue.push<output_event>(static_cast<input_event*>(e.get())->input);
    });
    dispatch.subscribe(event_type::OUTPUT, [](std::unique_ptr<event<event_type>>&& e){
        std::lock_guard<std::mutex> lck{cout_mutex};
        std::cout << "\t\t\toutput event after input: " << static_cast<output_event*>(e.get())->output << std::endl;
    });

    for (int i = 0; i < nr_of_threads; ++i) {
        generators.emplace_back(std::to_string(i), (i % 2) ? event_type::INPUT : event_type::MESSAGE, &queue, &dispatch);
    }

    std::for_each(generators.begin(), generators.end(), [](auto&& generator){
        generator.start();
    });

    while(queue.wait_for_event()) {
        auto event = queue.pop();
        dispatch.dispatch(std::move(event));
    }

    generators.clear();
    return 0;
}
