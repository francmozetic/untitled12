#include <condition_variable>
#include <deque>
#include <mutex>

class notification_queue {
    std::deque<std::function<void()>> _q;
    std::mutex _mutex;
    std::condition_variable _ready;

public:
    void pop(std::function<void()>& x) {
        std::unique_lock<std::mutex> lock{_mutex};
        while (_q.empty()) _ready.wait(lock);
        x = std::move(_q.front());
        _q.pop_front();
    }
};
