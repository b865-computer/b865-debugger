#include "Event.h"

void EventLoop::post(Event e)
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(e));
    }
    cv.notify_one();
}

void EventLoop::run()
{
    while (running)
    {
        Event e;
        {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]
                    { return !queue.empty() || !running; });
            if (!running)
                break;
            e = std::move(queue.front());
            queue.pop();
        }
        e(); // execute event
    }
}

void EventLoop::stop()
{
    running = false;
    cv.notify_all();
}