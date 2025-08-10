#pragma once
#ifndef _EVENT_H_
#define _EVENT_H_

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>
#include <iostream>

class EventLoop {
public:
    using Event = std::function<void()>;

    void post(Event e);
    void run();
    void stop();

private:
    std::queue<Event> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool running = true;
};

#endif // _EVENT_H_