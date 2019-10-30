#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

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

    template<typename F>
    void push(F&& f) {
        {
            std::unique_lock<std::mutex> lock{_mutex};
            _q.emplace_back(std::forward<F>(f));
        }
        _ready.notify_one();
    }
};

// Building a task system
class task_system {
    const unsigned _count = std::thread::hardware_concurrency();
    std::vector<std::thread> _threads;
    notification_queue _q;

    void run(unsigned i) {
        while (true) {
            std::function<void()> f;
            _q.pop(f);
            f();
        }
    }

public:
    task_system() {
        for (unsigned n=0; n!=_count; ++n) {
            _threads.emplace_back([&, n](){ run(n); });
        }
    }
    ~task_system() {
        for (auto& e : _threads) {
            e.join();
        }
    }



};
