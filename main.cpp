#include "dispatch.hpp"
#include "eventqueue.hpp"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mutex>
#include <csignal>
#include <string>
#include <thread>
#include <unistd.h>

namespace {
std::mutex mutex{};

constexpr int sec_to_micro(int sec) {
    return sec * 1000 * 1000;
}

}

namespace thread1 {
bool end = false;
struct message_event : public event_queue::event {
    message_event(std::string_view m)
    : event{event_queue::event_type::MESSAGE}, message{m.begin(), m.end()}
    {}
    std::string message;
};

void loop(event_queue* queue) {
    while(!end) {
        static int cnt{0};
        std::string message {"message_"};
        message += std::to_string(++cnt);
        queue->push<message_event>(message);
        usleep(sec_to_micro(1) + std::rand() % sec_to_micro(3));
    }
}

void handle(event_queue::event* e) {
    std::lock_guard<std::mutex> lock{mutex};
    std::cout << "thread1 event handler: " << static_cast<message_event*>(e)->message << std::endl;
}

void handle2(event_queue::event* e) {
    std::lock_guard<std::mutex> lock{mutex};
    std::cout << "thread1 event handler 2: " << static_cast<message_event*>(e)->message << std::endl;
}
}

namespace thread2 {
bool end = false;
struct input_event : public event_queue::event {
    input_event(int i)
    : event{event_queue::event_type::INPUT}, input{i}
    {}
    int input;
};

void loop(event_queue* queue) {
    while(!end) {
        static int cnt{0};
        queue->push<input_event>(++cnt);
        usleep(sec_to_micro(1) + std::rand() % sec_to_micro(3));
    }
}

void handle(event_queue::event* e) {
    std::lock_guard<std::mutex> lock{mutex};
    std::cout << "\t\tthread2 event handler: " << static_cast<input_event*>(e)->input << std::endl;
}
}

int main() {
    auto interrupt_handler = [](int s){
        if (s == SIGINT) {
            thread1::end = true;
            thread2::end = true;
            exit(0);
        }
    };
    signal(SIGINT, interrupt_handler);

    std::srand(std::time(nullptr));

    event_queue queue{};
    dispatcher dispatch{};

    std::thread t1{thread1::loop, &queue};
    std::thread t2{thread2::loop, &queue};

    dispatch.subscribe(event_queue::event_type::MESSAGE, thread1::handle);
    dispatch.subscribe(event_queue::event_type::MESSAGE, thread1::handle2);
    dispatch.subscribe(event_queue::event_type::INPUT, thread2::handle);

    while(true) {
        while (!queue.empty()) {
            auto event = queue.pop();
            dispatch.dispatch(event.get());
        }
        usleep(10000);
    }

    t1.join();
    t2.join();

    return 0;
}
